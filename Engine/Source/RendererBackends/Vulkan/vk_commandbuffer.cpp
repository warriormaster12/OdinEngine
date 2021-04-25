#include "Include/vk_commandbuffer.h"
#include "Include/vk_device.h"
#include "Include/vk_swapchain.h"
#include "Include/vk_init.h"


#include "function_queuer.h"
#include "vk_check.h"
#include "logger.h"
#include "window_handler.h"


FunctionQueuer commandDeletionQueue;
VkResult drawResult;


void VkCommandbufferManager::InitCommands()
{
    //create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(VkDeviceManager::GetGraphicsQueueFamily(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++) {

	
		VK_CHECK(vkCreateCommandPool(VkDeviceManager::GetDevice(), &commandPoolInfo, nullptr, &frames[i].commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(frames[i].commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(VkDeviceManager::GetDevice(), &cmdAllocInfo, &frames[i].mainCommandBuffer));
		commandDeletionQueue.PushFunction([=]() {
			vkDestroyCommandPool(VkDeviceManager::GetDevice(), frames[i].commandPool, nullptr);
		});
	}
	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(VkDeviceManager::GetGraphicsQueueFamily());
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(VkDeviceManager::GetDevice(), &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
	commandDeletionQueue.PushFunction([=]() {
		vkDestroyCommandPool(VkDeviceManager::GetDevice(), uploadContext.commandPool, nullptr);
	});

    ENGINE_CORE_INFO("vulkan commandbuffer created");
}

void VkCommandbufferManager::InitSyncStructures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		VK_CHECK(vkCreateFence(VkDeviceManager::GetDevice(), &fenceCreateInfo, nullptr, &frames[i].renderFence));

		//enqueue the destruction of the fence
		commandDeletionQueue.PushFunction([=]() {
			vkDestroyFence(VkDeviceManager::GetDevice(), frames[i].renderFence, nullptr);
			});


		VK_CHECK(vkCreateSemaphore(VkDeviceManager::GetDevice(), &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
		VK_CHECK(vkCreateSemaphore(VkDeviceManager::GetDevice(), &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));

		//enqueue the destruction of semaphores
		commandDeletionQueue.PushFunction([=]() {
			vkDestroySemaphore(VkDeviceManager::GetDevice(), frames[i].presentSemaphore, nullptr);
			vkDestroySemaphore(VkDeviceManager::GetDevice(), frames[i].renderSemaphore, nullptr);
			});
		
	}
	 VkFenceCreateInfo uploadFenceCreateInfo = vkinit::FenceCreateInfo();

	VK_CHECK(vkCreateFence(VkDeviceManager::GetDevice(), &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));

	commandDeletionQueue.PushFunction([=]() {
		vkDestroyFence(VkDeviceManager::GetDevice(), uploadContext.uploadFence, nullptr);
	});

    ENGINE_CORE_INFO("vulkan sync structure created");
}

void VkCommandbufferManager::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(uploadContext.commandPool, 1);

    VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(VkDeviceManager::GetDevice(), &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(VkDeviceManager::GetGraphicsQueue(), 1, &submit, uploadContext.uploadFence));

	vkWaitForFences(VkDeviceManager::GetDevice(), 1, &uploadContext.uploadFence, true, 9999999999);
	vkResetFences(VkDeviceManager::GetDevice(), 1, &uploadContext.uploadFence);

    //clear the command pool. This will free the command buffer too
	vkResetCommandPool(VkDeviceManager::GetDevice(), uploadContext.commandPool, 0);
}

void VkCommandbufferManager::CleanUpCommands()
{
    commandDeletionQueue.Flush();
    ENGINE_CORE_INFO("vulkan commands cleaned");
}

void VkCommandbufferManager::BeginCommands(std::function<void()> recreateSwapchain)
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(VkDeviceManager::GetDevice(), 1, &GetCurrentFrame().renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(VkDeviceManager::GetDevice(), 1, &GetCurrentFrame().renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0));

	//request image from the swapchain
	drawResult = vkAcquireNextImageKHR(VkDeviceManager::GetDevice(), VkSwapChainManager::GetSwapchain(), 1000000000, GetCurrentFrame().presentSemaphore, nullptr, &imageIndex);
	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
        return;
	} else if (drawResult != VK_SUCCESS && drawResult != VK_SUBOPTIMAL_KHR) {
		ENGINE_CORE_ERROR("failed to acquire swap chain image!");
		abort();
	}
	//naming it cmd for shorter writing
	cmd = GetCurrentFrame().mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
}

void VkCommandbufferManager::EndCommands(std::function<void()> recreateSwapchain)
{
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &GetCurrentFrame().presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &GetCurrentFrame().renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(VkDeviceManager::GetGraphicsQueue(), 1, &submit, GetCurrentFrame().renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();

	presentInfo.pSwapchains = &VkSwapChainManager::GetSwapchain();
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &imageIndex;

	drawResult = vkQueuePresentKHR(VkDeviceManager::GetGraphicsQueue(), &presentInfo);

	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR || drawResult == VK_SUBOPTIMAL_KHR || windowHandler.frameBufferResized == true) {
		windowHandler.frameBufferResized = false;
		recreateSwapchain();
		
	} else if (drawResult != VK_SUCCESS) {
		ENGINE_CORE_ERROR("failed to present swap chain image!");
		abort();
	}


	//increase the number of frames drawn
	frameNumber++;
}