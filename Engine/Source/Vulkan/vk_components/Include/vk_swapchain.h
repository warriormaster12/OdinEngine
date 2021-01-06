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
        SwapChain(VkPhysicalDevice& chosenGPU, VkDevice& device, VmaAllocator& allocator,DeletionQueue& refDeletionQueue);
        VkSwapchainKHR _swapchain; // from other articles

        // image format expected by the windowing system
        VkFormat _swapchainImageFormat; 	
        
        //array of images from the swapchain
        std::vector<VkImage> _swapchainImages;

        //array of image-views from the swapchain
        std::vector<VkImageView> _swapchainImageViews;

        VkExtent2D _windowExtent{ 1700 , 900 };
        VkSurfaceKHR _surface; // Vulkan window surface

        VkImageView _depthImageView;
        AllocatedImage _depthImage;

        //the format for the depth image
        VkFormat _depthFormat;
        void init_swapchain();
    private: 
        VkPhysicalDevice* _chosenGPU;
        VkDevice* _device;
        VmaAllocator* _allocator;
        DeletionQueue* _refDeletionQueue;
    };
}
