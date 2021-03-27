#pragma once 

#include "vk_types.h"
#include "vk_swapchain.h"
#include "window_handler.h"




class VkDeviceManager
{
public: 
    static void InitVulkan(SwapChain& swapChainObj, WindowHandler& windowHandler);

    static VkInstance& GetInstance() { return instance; }
	static VkPhysicalDevice& GetPhysicalDevice() { return chosenGPU; }
	static VkDevice& GetDevice() { return device; }
    static VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    static uint32_t& GetGraphicsQueueFamily() { return graphicsQueueFamily; }
    static const VmaAllocator& GetAllocator() { return allocator; }
    static VkDebugUtilsMessengerEXT& GetDebugMessenger() { return debugMessenger; }

private:
    inline static VkInstance instance;
    inline static VkDebugUtilsMessengerEXT debugMessenger;
    inline static VkPhysicalDevice chosenGPU;
    inline static VkDevice device;
    inline static VkPhysicalDeviceProperties gpuProperties;

    inline static VkQueue graphicsQueue;
    inline static uint32_t graphicsQueueFamily;

    inline static VmaAllocator allocator; //vma lib allocator

};



