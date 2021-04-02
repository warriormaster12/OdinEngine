#include "Include/vk_device.h"
#include "logger.h"
//bootstrap library
#include "VkBootstrap.h"



constexpr bool bUseValidationLayers = true;


void VkDeviceManager::InitVulkan(SwapChain& swapChainObj, WindowHandler& windowHandler)
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

	glfwCreateWindowSurface(instance, windowHandler.p_window, nullptr, &swapChainObj.surface);
	

	//use vkbootstrap to select a gpu. 
	//We want a gpu that can write to the GLFW surface and supports vulkan 1.2
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	VkPhysicalDeviceFeatures feats{};

	//feats.pipelineStatisticsQuery = true;
	feats.multiDrawIndirect = true;
	feats.drawIndirectFirstInstance = true;
	feats.alphaToOne = false;
	feats.depthClamp = true;

	
	feats.samplerAnisotropy = true;
	

	VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
	descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

	descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
	descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
	descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

	selector.set_required_features(feats);
	selector.add_desired_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(swapChainObj.surface)
		.select()
		.value();
	//create the final vulkan device

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.add_pNext(&descriptorIndexingFeatures).build().value();
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


	


