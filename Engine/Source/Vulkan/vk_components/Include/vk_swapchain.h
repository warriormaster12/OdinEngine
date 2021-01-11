#pragma once 

#include "../../Include/vk_types.h"
#include "../../Include/vk_init.h"
#include "vk_deletionqueue.h"
#include "vk_check.h"
#include <iostream>

namespace vkcomponent
{
    class SwapChain
    {
    public:
        SwapChain(VkPhysicalDevice& chosenGPU, VkDevice& device, VmaAllocator& allocator);
        VkSwapchainKHR _swapchain; // from other articles

        // image format expected by the windowing system
        VkFormat _swapchainImageFormat; 	
        
        //array of images from the swapchain
        std::vector<VkImage> _swapchainImages;

        //array of image-views from the swapchain
        std::vector<VkImageView> _swapchainImageViews;

        VkExtent2D _windowExtent{};
        VkSurfaceKHR _surface; // Vulkan window surface

        VkImageView _depthImageView;
        AllocatedImage _depthImage;

        //the format for the depth image
        VkFormat _depthFormat;
        void init_swapchain();
        void destroySwapChain();
    private: 
        VkPhysicalDevice* _chosenGPU;
        VkDevice* _device;
        VmaAllocator* _allocator;
    };
}
