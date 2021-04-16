#pragma once 

#include "vk_types.h"






class VkDeviceManager
{
public: 
    static void InitDevice();
    static void DestroyDevice();

    static VkInstance& GetInstance() { return instance; }
	static VkPhysicalDevice& GetPhysicalDevice() { return chosenGPU; }
	static VkDevice& GetDevice() { return device; }
    static VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    static VkSurfaceKHR& GetSurface() { return surface; }
    static uint32_t& GetGraphicsQueueFamily() { return graphicsQueueFamily; }
    static const VmaAllocator& GetAllocator() { return allocator; }
    static VkDebugUtilsMessengerEXT& GetDebugMessenger() { return debugMessenger; }

    static VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() {return gpuProperties; }

private:
    inline static VkInstance instance;
    inline static VkDebugUtilsMessengerEXT debugMessenger;
    inline static VkPhysicalDevice chosenGPU;
    inline static VkDevice device;
    inline static VkSurfaceKHR surface;
    inline static VkPhysicalDeviceProperties gpuProperties;

    inline static VkQueue graphicsQueue;
    inline static uint32_t graphicsQueueFamily;

    inline static VmaAllocator allocator; //vma lib allocator

};



