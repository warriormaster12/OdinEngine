#pragma once 

#include "../../Include/vk_types.h"
#include "vk_deletionqueue.h"
#include <iostream>

namespace vkcomponent
{
    class SwapChain
    {
    public:
        VkSwapchainKHR _swapchain; // from other articles

        // image format expected by the windowing system
        VkFormat _swapchainImageFormat; 	
        
        //array of images from the swapchain
        std::vector<VkImage> _swapchainImages;

        //array of image-views from the swapchain
        std::vector<VkImageView> _swapchainImageViews;

        VkExtent2D _windowExtent{ 1700 , 900 };
        VkSurfaceKHR _surface; // Vulkan window surface
        void init_swapchain(VkPhysicalDevice _chosenGPU, VkDevice _device, DeletionQueue& _refDeletionQueue);
    };
}
