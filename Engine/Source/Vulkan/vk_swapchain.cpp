#include "Include/vk_swapchain.h"
#include "vk_check.h"
#include "VkBootstrap.h"
#include "Include/vk_device.h"


void SwapChain::InitSwapchain(GLFWwindow* p_window, FunctionQueuer& deletionQueue)
{
	int width;
	int height;
	glfwGetFramebufferSize(p_window, &width, &height);
    vkb::SwapchainBuilder swapchainBuilder{VkDeviceManager::GetPhysicalDevice(),VkDeviceManager::GetDevice(),surface };
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
		.set_desired_extent(static_cast<uint32_t>(width), static_cast<uint32_t>(height))
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
	swapchainImageFormat = vkbSwapchain.image_format;

	//we get actual resolution of the displayed content
	actualExtent = vkbSwapchain.extent;
	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		actualExtent.width,
		actualExtent.height,
		1
	};

	//hardcoding the depth format to 32 bit float
	depthFormat = VK_FORMAT_D32_SFLOAT;

	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(VkDeviceManager::GetAllocator(), &dimg_info, &dimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = vkinit::ImageViewCreateInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);;

	VK_CHECK(vkCreateImageView(VkDeviceManager::GetDevice(), &dview_info, nullptr, &depthImageView));

	deletionQueue.PushFunction([=]()
	{
		vkDestroyImageView(VkDeviceManager::GetDevice(), depthImageView, nullptr);
		vmaDestroyImage(VkDeviceManager::GetAllocator(), depthImage.image, depthImage.allocation);
		for (int i = 0; i < swapchainImageViews.size(); i++)
		{
			vkDestroyImageView(VkDeviceManager::GetDevice(), swapchainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(VkDeviceManager::GetDevice(),swapchain, nullptr);
	});

}



