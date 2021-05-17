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
    static inline VkInstance instance;
    static inline VkDebugUtilsMessengerEXT debugMessenger;
    static inline VkPhysicalDevice chosenGPU;
    static inline VkDevice device;
    static inline VkSurfaceKHR surface;
    static inline VkPhysicalDeviceProperties gpuProperties;

    static inline VkQueue graphicsQueue;
    static inline uint32_t graphicsQueueFamily;

    static inline VmaAllocator allocator; //vma lib allocator

};



