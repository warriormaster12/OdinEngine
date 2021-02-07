#include "Include/vk_renderer.h"
#include "Include/vk_types.h"
#include "Include/vk_init.h"
#include "vk_utils.h"
#include "vk_check.h"
#include "vk_pipelinebuilder.h"
#include "vk_shaderhandler.h"
#include "vk_textures.h"
#include "asset_builder.h"
#include "imgui_impl_vulkan.h"
#include "Imgui_layer.h"


//bootstrap library
#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "logger.h"

vkcomponent::PipelineBuilder pipelineBuilder;
VkCommandBuffer cmd;
uint32_t swapchainImageIndex;
VkResult drawResult;

// Utility (pure) functions are put in an anonymous namespace

namespace {
    void UploadCameraData(const VmaAllocator& allocator, const VmaAllocation& allocation, const Camera& cam)
    {
        GPUCameraData camData;
        camData.view = cam.GetViewMatrix();
        camData.proj = cam.GetProjectionMatrix(false);
        camData.viewproj = camData.proj * camData.view;
        camData.camPos = glm::vec4(glm::vec3(cam.position), 0.0f);

		UploadSingleData(allocator, allocation, camData);
    }

    void UploadSceneData(const VmaAllocator& allocator, const VmaAllocation& allocation, const GPUSceneData& data, size_t offset)
    {
		UploadSingleData(allocator, allocation, data, offset);
    }

    void UploadObjectData(const VmaAllocator& allocator, const VmaAllocation& allocation, const std::vector<RenderObject>& objects)
    {
        std::vector<GPUObjectData> data;
        data.reserve(objects.size());
        for (const RenderObject& obj : objects)
        {
            GPUObjectData objData;
            objData.modelMatrix = obj.transformMatrix;
            data.push_back(objData);
        }

		UploadVectorData(allocator, allocation, data);
    }

    void UploadMaterialData(const VmaAllocator& allocator, std::vector<Material>& materials)
    {
        for(auto material : materials)
        {
            GPUMaterialData materialData;
            materialData.albedo = material.albedo;
            materialData.metallic = glm::vec4(glm::vec3(material.metallic), 0.0f);
            materialData.roughness = glm::vec4(glm::vec3(material.roughness), 0.0f);
            materialData.ao = glm::vec4(glm::vec3(material.ao), 0.0f);
			materialData.emissionColor = glm::vec4(glm::vec3(material.emissionColor), 1.0f);
			materialData.emissionPower = glm::vec4(glm::vec3(materialData.emissionPower), 0.0f);

			UploadSingleData(allocator, material.buffer.allocation, materialData);
        }
    }

    void UploadDrawCalls(const VmaAllocator& allocator, const VmaAllocation& allocation, const std::vector<RenderObject>& objects)
    {
        std::vector<VkDrawIndirectCommand> commands;
        commands.reserve(objects.size());
        for (size_t i = 0; i < objects.size(); ++i)
        {
            VkDrawIndirectCommand cmd;
            cmd.vertexCount = objects[i].p_mesh->vertices.size();
            cmd.instanceCount = 1;
            cmd.firstVertex = 0;
            cmd.firstInstance = i;
            commands.push_back(cmd);
        }

		UploadVectorData(allocator, allocation, commands);
    }

    std::vector<DrawCall> BatchDrawCalls(const std::vector<RenderObject>& objects, const DescriptorSetData& descriptorSets)
    {
        std::vector<DrawCall> batch;

		Mesh* pLastMesh = nullptr;
		Material* pLastMaterial = nullptr;

        for (size_t i = 0; i < objects.size(); ++i)
        {
			bool isSameMesh = objects[i].p_mesh == pLastMesh;
			bool isSameMaterial = objects[i].p_material == pLastMaterial;

			if (i == 0 || !isSameMesh || !isSameMaterial) {
                DrawCall dc;
                dc.pMesh = objects[i].p_mesh;
                dc.pMaterial = objects[i].p_material;
                dc.descriptorSets = descriptorSets;
                dc.transformMatrix = objects[0].transformMatrix;
                dc.index = i;
                dc.count = 1;
                batch.push_back(dc);

				pLastMesh = objects[i].p_mesh;
				pLastMaterial = objects[i].p_material;
			} else {
				++batch.back().count;
			}
        }

        return batch;
    }

    void BindDynamicStates()
    {
        vkCmdSetViewport(cmd, 0, 1, &pipelineBuilder.viewport);
        vkCmdSetScissor(cmd, 0, 1, &pipelineBuilder.scissor);
        float blendConstant[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        vkCmdSetBlendConstants(cmd, blendConstant);
    }

    void BindMaterial(Material* material, const DescriptorSetData& descriptorSets)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &descriptorSets.uniform, 1, &descriptorSets.uniformOffset);

        //object data descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 1, 1, &descriptorSets.object, 0, &descriptorSets.objectOffset);
        //texture + material descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 2, 1, &material->materialSet, 0, nullptr);
    }

    void BindMesh(Mesh* mesh)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vertexBuffer.buffer, &offset);
    }

	void IssueDrawCalls(const VkCommandBuffer& cmd, const VkBuffer& drawCommandBuffer, const std::vector<DrawCall>& drawCalls)
	{
		for (const DrawCall& dc : drawCalls)
		{
			BindDynamicStates();

			BindMaterial(dc.pMaterial, dc.descriptorSets);
			BindMesh(dc.pMesh);

			uint32_t stride = sizeof(VkDrawIndirectCommand);
			uint32_t offset = dc.index * stride;

			vkCmdDrawIndirect(cmd, drawCommandBuffer, offset, dc.count, stride);
		}
	}
}


constexpr bool bUseValidationLayers = true;
void VulkanRenderer::Init(WindowHandler& windowHandler)
{
	p_windowHandler = &windowHandler;
	InitVulkan();
	swapChainObj.InitSwapchain(p_windowHandler->p_window);

	InitDefaultRenderpass();

	InitFramebuffers();

	InitCommands();

	InitSyncStructures();

	InitDescriptors();

	InitPipelines();

	InitScene();

	camera.position = { 0.f,0.f,10.f };

	ENGINE_CORE_INFO("vulkan intialized");
}
void VulkanRenderer::CleanUp()
{	
		
	vkDeviceWaitIdle(device);
	
	swapDeletionQueue.Flush();
	mainDeletionQueue.Flush();

	vkDestroySurfaceKHR(instance, swapChainObj.surface, nullptr);

	vmaDestroyAllocator(allocator);
	vkDestroyDevice(device, nullptr);
	vkb::destroy_debug_utils_messenger(instance, debugMessenger);
	vkDestroyInstance(instance, nullptr);

	ENGINE_CORE_INFO("vulkan destroyed");
}


void VulkanRenderer::BeginDraw()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &GetCurrentFrame().renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &GetCurrentFrame().renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0));

	//request image from the swapchain
	drawResult = vkAcquireNextImageKHR(device, swapChainObj.swapchain, 1000000000, GetCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex);
	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
	} else if (drawResult != VK_SUCCESS && drawResult != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//naming it cmd for shorter writing
	cmd = GetCurrentFrame().mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(renderPass, swapChainObj.actualExtent, framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];
	
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

}
void VulkanRenderer::EndDraw()
{
	//finalize the render pass
	vkCmdEndRenderPass(cmd);
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
	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, GetCurrentFrame().renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();

	presentInfo.pSwapchains = &swapChainObj.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	drawResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);

	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR || drawResult == VK_SUBOPTIMAL_KHR || p_windowHandler->frameBufferResized == true) {
		p_windowHandler->frameBufferResized = false;
		RecreateSwapchain();
	} else if (drawResult != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}


	//increase the number of frames drawn
	frameNumber++;
}

FrameData& VulkanRenderer::GetCurrentFrame()
{
	return frames[frameNumber % FRAME_OVERLAP];
}


FrameData& VulkanRenderer::GetLastFrame()
{
	return frames[(frameNumber -1) % 2];
}


void VulkanRenderer::InitVulkan()
{
	vkb::InstanceBuilder builder;

	//make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 1, 0)		
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//grab the instance 
	instance = vkb_inst.instance;
	debugMessenger = vkb_inst.debug_messenger;

	glfwCreateWindowSurface(instance, p_windowHandler->p_window, nullptr, &swapChainObj.surface);
	

	//use vkbootstrap to select a gpu. 
	//We want a gpu that can write to the SDL surface and supports vulkan 1.2
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	VkPhysicalDeviceFeatures feats{};

	//feats.pipelineStatisticsQuery = true;
	feats.multiDrawIndirect = true;
	feats.drawIndirectFirstInstance = true;
	//feats.samplerAnisotropy = true;
	selector.set_required_features(feats);

	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(swapChainObj.surface)
		.select()
		.value();
	//create the final vulkan device

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();
	// Get the VkDevice handle used in the rest of a vulkan application
	device = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = chosenGPU;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	vmaCreateAllocator(&allocatorInfo, &allocator);

	

	vkGetPhysicalDeviceProperties(chosenGPU, &gpuProperties);
	ENGINE_CORE_INFO(physicalDevice.properties.deviceName);
	ENGINE_CORE_INFO("The gpu has a minimum buffer alignement of {0}", gpuProperties.limits.minUniformBufferOffsetAlignment);
}

void VulkanRenderer::RecreateSwapchain()
{	
	vkDeviceWaitIdle(device);
	swapDeletionQueue.Flush();
	int width = 0, height = 0;
	glfwGetFramebufferSize(p_windowHandler->p_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(p_windowHandler->p_window, &width, &height);
		glfwWaitEvents();
	}

	swapChainObj.InitSwapchain(p_windowHandler->p_window);

	InitFramebuffers();	

	//update parts of the pipeline that change dynamically
	pipelineBuilder.viewport.width = (float)swapChainObj.actualExtent.width;
	pipelineBuilder.viewport.height = (float)swapChainObj.actualExtent.height;
	pipelineBuilder.scissor.extent = swapChainObj.actualExtent;
}

void VulkanRenderer::InitDefaultRenderpass()
{
	//we define an attachment description for our main color image
	//the attachment is loaded as "clear" when renderpass start
	//the attachment is stored when renderpass ends
	//the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
	//we dont care about stencil, and dont use multisampling

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swapChainObj.swapchainImageFormat;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = swapChainObj.depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;
	
	VK_CHECK(vkCreateRenderPass(device, &render_pass_info, nullptr, &renderPass));
	mainDeletionQueue.PushFunction([=]() {
		vkDestroyRenderPass(device, renderPass, nullptr);
	});
}

void VulkanRenderer::InitFramebuffers()
{
	const uint32_t swapchain_imagecount = swapChainObj.swapchainImageViews.size();
	framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) {
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		VkFramebufferCreateInfo fb_info = vkinit::FramebufferCreateInfo(renderPass, swapChainObj.actualExtent);
		std::array <VkImageView, 2> attachments = {swapChainObj.swapchainImageViews[i], swapChainObj.depthImageView};


		fb_info.attachmentCount = attachments.size();
		fb_info.pAttachments = attachments.data();
		VK_CHECK(vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffers[i]));
		swapDeletionQueue.PushFunction([=]() {
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		});
	}
}

void VulkanRenderer::InitCommands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++) {

	
		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(frames[i].commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer));
		mainDeletionQueue.PushFunction([=]() {
			vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
		});
	}
	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(graphicsQueueFamily);
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(device, &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
	mainDeletionQueue.PushFunction([=]() {
		vkDestroyCommandPool(device, uploadContext.commandPool, nullptr);
	});
}

void VulkanRenderer::InitSyncStructures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));

		//enqueue the destruction of the fence
		mainDeletionQueue.PushFunction([=]() {
			vkDestroyFence(device, frames[i].renderFence, nullptr);
			});


		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));

		//enqueue the destruction of semaphores
		mainDeletionQueue.PushFunction([=]() {
			vkDestroySemaphore(device, frames[i].presentSemaphore, nullptr);
			vkDestroySemaphore(device, frames[i].renderSemaphore, nullptr);
			});
		
	}
	 VkFenceCreateInfo uploadFenceCreateInfo = vkinit::FenceCreateInfo();

	VK_CHECK(vkCreateFence(device, &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));

	mainDeletionQueue.PushFunction([=]() {
		vkDestroyFence(device, uploadContext.uploadFence, nullptr);
	});

}


void VulkanRenderer::InitPipelines()
{
	VkShaderModule texturedMeshShader;
	if (!vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_pbr_lit.frag").c_str(), &texturedMeshShader, device))
	{
		ENGINE_CORE_ERROR("Error when building the textured mesh shader");
	}

	VkShaderModule meshVertShader;
	if (!vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_triangle.vert").c_str(), &meshVertShader, device))
	{
		ENGINE_CORE_ERROR("Error when building the mesh vertex shader module");
	}

	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	pipelineBuilder.shaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
	pipelineBuilder.shaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

	//we start from  the normal mesh layout
	VkPipelineLayoutCreateInfo textured_pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
		
	VkDescriptorSetLayout texturedSetLayouts[] = { globalSetLayout, objectSetLayout, materialTextureSetLayout };
	textured_pipeline_layout_info.setLayoutCount = 3;
	textured_pipeline_layout_info.pSetLayouts = texturedSetLayouts;

	VkPipelineLayout texturedPipeLayout;
	VK_CHECK(vkCreatePipelineLayout(device, &textured_pipeline_layout_info, nullptr, &texturedPipeLayout));

	//hook the push constants layout
	pipelineBuilder.pipelineLayout = texturedPipeLayout;

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder.viewport.x = 0.0f;
	pipelineBuilder.viewport.y = 0.0f;
	pipelineBuilder.viewport.width = (float)swapChainObj.actualExtent.width;
	pipelineBuilder.viewport.height = (float)swapChainObj.actualExtent.height;
	pipelineBuilder.viewport.minDepth = 0.0f;
	pipelineBuilder.viewport.maxDepth = 1.0f;

	pipelineBuilder.scissor.offset = { 0, 0 };
	pipelineBuilder.scissor.extent = swapChainObj.actualExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();


	//default depthtesting
	pipelineBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the mesh pipeline

	VertexInputDescription vertexDescription = Vertex::GetVertexDescription();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	
	//build the mesh triangle pipeline
	pipelineBuilder.pipelineLayout = texturedPipeLayout;
	VkPipeline texPipeline = pipelineBuilder.BuildPipeline(device, renderPass);
	CreateMaterial(texPipeline, texturedPipeLayout, "texturedmesh");
	CreateMaterial(texPipeline, texturedPipeLayout, "texturedmesh2");
	CreateMaterial(texPipeline, texturedPipeLayout, "texturedmesh3");
	CreateMaterial(texPipeline, texturedPipeLayout, "DamagedHelmetMat");

	vkDestroyShaderModule(device, meshVertShader, nullptr);
	vkDestroyShaderModule(device, texturedMeshShader, nullptr);

	mainDeletionQueue.PushFunction([=]() {
		vkDestroyPipeline(device, texPipeline, nullptr);
		vkDestroyPipelineLayout(device, texturedPipeLayout, nullptr);
	});
}


void VulkanRenderer::UploadMesh(Mesh& mesh)
{
	const size_t BUFFER_SIZE = mesh.vertices.size() * sizeof(Vertex);

	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = BUFFER_SIZE;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(allocator, &stagingBuffer, info);
	}

	//copy vertex data
	UploadVectorData(allocator, stagingBuffer.allocation, mesh.vertices);

	{
		CreateBufferInfo info;
		info.allocSize = BUFFER_SIZE;
		info.bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		CreateBuffer(allocator, &mesh.vertexBuffer, info);
	}

	//add the destruction of triangle mesh buffer to the deletion queue
	mainDeletionQueue.PushFunction([=]() {
		vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
	});

	ImmediateSubmit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = BUFFER_SIZE;
		vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, & copy);
	});

	vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

void VulkanRenderer::LoadImage(std::string texture_name, std::string texture_path)
{
	Texture inputTextures;
	const std::filesystem::path path = texture_path;
	const std::filesystem::path bin_path = texture_path + ".bin";
	
	asset_builder::convert_image(path,bin_path);
	vkcomponent::LoadImageFromAsset(*this, (texture_path + ".bin").c_str(), &inputTextures.image);
	
	VkImageViewCreateInfo imageinfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, inputTextures.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(device, &imageinfo, nullptr, &inputTextures.imageView);

	_loadedTextures[texture_name] = inputTextures;

	mainDeletionQueue.PushFunction([=]() {
		vkDestroyImageView(device, inputTextures.imageView, nullptr);
		vmaDestroyImage(allocator, inputTextures.image.image, inputTextures.image.allocation);
	});

}



Material* VulkanRenderer::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	materials[name] = mat;
	materialList.push_back(name);

	p_descriptorAllocator->Allocate(&materials[name].materialSet, materialTextureSetLayout);

	{
		CreateBufferInfo info;
		info.allocSize = sizeof(GPUMaterialData);
		info.bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		CreateBuffer(allocator, &materials[name].buffer, info);
	}

	VkDescriptorBufferInfo materialBufferInfo;
	materialBufferInfo.buffer = materials[name].buffer.buffer;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(GPUMaterialData);

	VkWriteDescriptorSet objectFragWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, materials[name].materialSet, &materialBufferInfo, 0);
	vkUpdateDescriptorSets(device, 1, &objectFragWrite, 0, nullptr);
	mainDeletionQueue.PushFunction([=]()
	{
		vmaDestroyBuffer(allocator, materials[name].buffer.buffer, materials[name].buffer.allocation);
	});

	return &materials[name];
}

Material* VulkanRenderer::GetMaterial(const std::string& name)
{
	//search for the object, and return nullpointer if not found
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	}
	else {
		return &it->second;
	}
}





void VulkanRenderer::DrawObjects(const std::vector<RenderObject>& objects)
{
	size_t frameIndex = frameNumber % FRAME_OVERLAP;
	size_t uniformOffset = PadUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;

	// Static light data, can be moved away
	sceneParameters.lightData.lightPositions[0] = glm::vec4(glm::vec3(0.0f,  1.0f, 2.0f),0.0f);
	sceneParameters.lightData.lightColors[0] = glm::vec4(glm::vec3(1.0f),0.0f);
	sceneParameters.lightData.lightPositions[1] = glm::vec4(glm::vec3(0.0f,  4.0f, 8.0f),0.0f);
	sceneParameters.lightData.lightColors[1] = glm::vec4(glm::vec3(3.0f),0.0f);

	// Convert material ID to material
	// TODO: Handle nullptr material, apply default?
	std::vector<Material> materials;
	materials.reserve(materialList.size());
	for (const std::string& materialName : materialList)
		materials.push_back(*GetMaterial(materialName));

	// Store descriptor set data
	DescriptorSetData descriptorSets;
	descriptorSets.uniform = GetCurrentFrame().globalDescriptor;
	descriptorSets.uniformOffset = uniformOffset;
	descriptorSets.object = GetCurrentFrame().objectDescriptor;
	descriptorSets.objectOffset = 0;

	UploadCameraData(allocator, GetCurrentFrame().cameraBuffer.allocation, camera);
	UploadSceneData(allocator, sceneParameterBuffer.allocation, sceneParameters, uniformOffset);
	UploadObjectData(allocator, GetCurrentFrame().objectBuffer.allocation, objects);
	
	UploadMaterialData(allocator, materials);

	UploadDrawCalls(allocator, GetCurrentFrame().indirectDrawBuffer.allocation, objects);

	std::vector<DrawCall> drawCalls = BatchDrawCalls(objects, descriptorSets);
	IssueDrawCalls(cmd, GetCurrentFrame().indirectDrawBuffer.buffer, drawCalls);
}



void VulkanRenderer::InitScene()
{
	//TODO: move all of this into higher rendering level
	GetMaterial("texturedmesh")->albedo = glm::vec4(1.0f);
	GetMaterial("texturedmesh")->metallic = 1.0f;
	GetMaterial("texturedmesh")->roughness = 0.25f;
	GetMaterial("texturedmesh")->ao = 1.0f;
	GetMaterial("texturedmesh")->emissionColor = glm::vec3(0.0f);
	GetMaterial("texturedmesh")->emissionPower = 1.0f;

	GetMaterial("texturedmesh2")->albedo = glm::vec4(0.0f,0.0f,0.0f,1.0f);
	GetMaterial("texturedmesh2")->metallic = 0.5f;
	GetMaterial("texturedmesh2")->roughness = 0.5f;
	GetMaterial("texturedmesh2")->ao = 1.0f;
	GetMaterial("texturedmesh2")->emissionColor = glm::vec3(0.0f);
	GetMaterial("texturedmesh2")->emissionPower = 1.0f;

	GetMaterial("texturedmesh3")->albedo = glm::vec4(0.5f);
	GetMaterial("texturedmesh3")->metallic = 0.5f;
	GetMaterial("texturedmesh3")->roughness = 0.5f;
	GetMaterial("texturedmesh3")->ao = 1.0f;
	GetMaterial("texturedmesh3")->emissionColor = glm::vec3(0.0f);
	GetMaterial("texturedmesh3")->emissionPower = 1.0f;

	GetMaterial("DamagedHelmetMat")->albedo = glm::vec4(1.0f);
	GetMaterial("DamagedHelmetMat")->metallic = 0.8f;
	GetMaterial("DamagedHelmetMat")->roughness = 0.2f;
	GetMaterial("DamagedHelmetMat")->ao = 1.0f;
	GetMaterial("DamagedHelmetMat")->emissionColor = glm::vec3(1.0f);
	GetMaterial("DamagedHelmetMat")->emissionPower = 16.0f;
	
	//create a sampler for the texture
	VkSamplerCreateInfo samplerInfo = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST);

	VkSampler blockySampler;
	vkCreateSampler(device, &samplerInfo, nullptr, &blockySampler);
	LoadImage("empty", "");
	//empty texture slots
	CreateTexture("texturedmesh2", "empty", blockySampler);
	CreateTexture("texturedmesh2", "empty", blockySampler, 2);
	CreateTexture("texturedmesh", "empty", blockySampler, 2);
	CreateTexture("texturedmesh3", "empty", blockySampler, 2);
	CreateTexture("texturedmesh2", "empty", blockySampler, 3);
	CreateTexture("texturedmesh", "empty", blockySampler, 3);
	CreateTexture("texturedmesh3", "empty", blockySampler, 3);
	CreateTexture("texturedmesh2", "empty", blockySampler, 4);
	CreateTexture("texturedmesh", "empty", blockySampler, 4);
	CreateTexture("texturedmesh3", "empty", blockySampler, 4);

	LoadImage("empire_diffuse", "EngineAssets/Textures/lost_empire-RGBA.png");
	CreateTexture("texturedmesh", "empire_diffuse", blockySampler);
	LoadImage("vikingroom_diffuse", "EngineAssets/Textures/viking_room.png");
	CreateTexture("texturedmesh3", "vikingroom_diffuse", blockySampler);

	//DamagedHelmet

	//diffuse
	LoadImage("DamagedHelmet_diffuse", "EngineAssets/DamagedHelmet/Default_albedo.jpg");
	CreateTexture("DamagedHelmetMat", "DamagedHelmet_diffuse", blockySampler);
	//ao
	LoadImage("DamagedHelmet_ao", "EngineAssets/DamagedHelmet/Default_AO.jpg");
	CreateTexture("DamagedHelmetMat", "DamagedHelmet_ao", blockySampler,2);
	//normal
	LoadImage("DamagedHelmet_normal", "EngineAssets/DamagedHelmet/Default_normal.jpg");
	CreateTexture("DamagedHelmetMat", "DamagedHelmet_normal", blockySampler,3);

	//emission
	LoadImage("DamagedHelmet_emission", "EngineAssets/DamagedHelmet/Default_emissive.jpg");
	CreateTexture("DamagedHelmetMat", "DamagedHelmet_emission", blockySampler,4);


	mainDeletionQueue.PushFunction([=]() {
		vkDestroySampler(device, blockySampler, nullptr);
	});
}


size_t VulkanRenderer::PadUniformBufferSize(size_t originalSize)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}


void VulkanRenderer::InitDescriptors()
{
	p_descriptorAllocator = new vkcomponent::DescriptorAllocator{};
	p_descriptorAllocator->Init(device);

	p_descriptorLayoutCache = new vkcomponent::DescriptorLayoutCache{};
	p_descriptorLayoutCache->Init(device);
	
	VkDescriptorSetLayoutBinding cameraBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	std::vector<VkDescriptorSetLayoutBinding> bindings = { cameraBind,sceneBind };
	VkDescriptorSetLayoutCreateInfo _set1 = vkinit::DescriptorLayoutInfo(bindings);
	globalSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set1);

	VkDescriptorSetLayoutBinding objectBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	std::vector<VkDescriptorSetLayoutBinding> objectBindings = { objectBind };
	VkDescriptorSetLayoutCreateInfo _set2 = vkinit::DescriptorLayoutInfo(objectBindings);
	objectSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set2);


	VkDescriptorSetLayoutBinding objectMaterialBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding diffuseTextureBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding aoTextureBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutBinding normalTextureBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	VkDescriptorSetLayoutBinding emissionTextureBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
	std::vector<VkDescriptorSetLayoutBinding> textureBindings = {objectMaterialBind, diffuseTextureBind, aoTextureBind, normalTextureBind, emissionTextureBind};
	VkDescriptorSetLayoutCreateInfo _set3 = vkinit::DescriptorLayoutInfo(textureBindings);
	materialTextureSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set3);

	{
		CreateBufferInfo info;
		info.allocSize = FRAME_OVERLAP * PadUniformBufferSize(sizeof(GPUSceneData));
		info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		CreateBuffer(allocator, &sceneParameterBuffer, info);
	}

    const int MAX_OBJECTS = 10000;
    const int MAX_COMMANDS = 1000;

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		frames[i].p_dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		frames[i].p_dynamicDescriptorAllocator->Init(device);
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].globalDescriptor, globalSetLayout);
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].objectDescriptor, objectSetLayout);

		{
			CreateBufferInfo info;
			info.allocSize = sizeof(GPUCameraData);
			info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(allocator, &frames[i].cameraBuffer, info);
		}

		{
			CreateBufferInfo info;
			info.allocSize = MAX_OBJECTS * sizeof(GPUObjectData);
			info.bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            CreateBuffer(allocator, &frames[i].objectBuffer, info);
		}

		{
			CreateBufferInfo info;
			info.allocSize = MAX_COMMANDS * sizeof(VkDrawIndirectCommand);
			info.bufferUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(allocator, &frames[i].indirectDrawBuffer, info);
		}

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = frames[i].cameraBuffer.buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = sceneParameterBuffer.buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

		VkWriteDescriptorSet cameraWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, frames[i].globalDescriptor,&cameraInfo,0);
		VkWriteDescriptorSet sceneWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, frames[i].globalDescriptor, &sceneInfo, 1);
		VkWriteDescriptorSet objectWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, frames[i].objectDescriptor, &objectBufferInfo, 0);

		std::vector <VkWriteDescriptorSet> setWrites = { cameraWrite, sceneWrite, objectWrite};
		vkUpdateDescriptorSets(device, setWrites.size(), setWrites.data(), 0, nullptr);

		mainDeletionQueue.PushFunction([=]()
		{
			vmaDestroyBuffer(allocator, frames[i].objectBuffer.buffer, frames[i].objectBuffer.allocation);
			vmaDestroyBuffer(allocator, frames[i].indirectDrawBuffer.buffer, frames[i].indirectDrawBuffer.allocation);
			vmaDestroyBuffer(allocator, frames[i].cameraBuffer.buffer, frames[i].cameraBuffer.allocation);
		});
	}

	mainDeletionQueue.PushFunction([=]() {
		vmaDestroyBuffer(allocator, sceneParameterBuffer.buffer, sceneParameterBuffer.allocation);
		for (auto& frame : frames)
		{
			frame.p_dynamicDescriptorAllocator->CleanUp();
		}
		p_descriptorAllocator->CleanUp();
		p_descriptorLayoutCache->CleanUp();
	});
	
}


void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(uploadContext.commandPool, 1);

    VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, uploadContext.uploadFence));

	vkWaitForFences(device, 1, &uploadContext.uploadFence, true, 9999999999);
	vkResetFences(device, 1, &uploadContext.uploadFence);

    //clear the command pool. This will free the command buffer too
	vkResetCommandPool(device, uploadContext.commandPool, 0);
}

void VulkanRenderer::CreateTexture(std::string materialName, std::string textureName, VkSampler& sampler, uint32_t binding /*= 1*/)
{
	Material* texturedMaterial = GetMaterial(materialName);

	//write to the descriptor set so that it points to our empire_diffuse texture
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = sampler;
	imageBufferInfo.imageView = _loadedTextures[textureName].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMaterial->materialSet, &imageBufferInfo, binding);
	
	vkUpdateDescriptorSets(device, 1, &outputTexture, 0, nullptr);
}
