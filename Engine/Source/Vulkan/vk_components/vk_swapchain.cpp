#include "Include/vk_swapchain.h"
#include "VkBootstrap.h"

vkcomponent::SwapChain::SwapChain(VkPhysicalDevice& chosenGPU, VkDevice& device, VmaAllocator& allocator,DeletionQueue& refDeletionQueue)
{
	_chosenGPU = &chosenGPU;
	_device = &device;
	_allocator = &allocator;
	_refDeletionQueue = &refDeletionQueue;
}

void vkcomponent::SwapChain::init_swapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{*_chosenGPU,*_device,_surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(_windowExtent.width, _windowExtent.height)
		.build()
		.value();

	//store swapchain and its related images
	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();

	_swapchainImageFormat = vkbSwapchain.image_format;

	_refDeletionQueue->push_function([=]() {
		vkDestroySwapchainKHR(*_device,_swapchain, nullptr);
	});

	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		_windowExtent.width,
		_windowExtent.height,
		1
	};

	//hardcoding the depth format to 32 bit float
	_depthFormat = VK_FORMAT_D32_SFLOAT;

	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(*_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);;

	VK_CHECK(vkCreateImageView(*_device, &dview_info, nullptr, &_depthImageView));

	//add to deletion queues
	_refDeletionQueue->push_function([=]() {
		vkDestroyImageView(*_device, _depthImageView, nullptr);
		vmaDestroyImage(*_allocator, _depthImage._image, _depthImage._allocation);
	});
}

