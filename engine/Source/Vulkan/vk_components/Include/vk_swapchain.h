#pragma once 

#include "../../Include/vk_types.h"
#include "../../Include/vk_init.h"
#include "vk_deletionqueue.h"
#include "vk_check.h"
#include <iostream>
#include <SDL.h>
#include <SDL_vulkan.h>

namespace vkcomponent
{
    class SwapChain
    {
    public:
        SwapChain(VkPhysicalDevice& chosenGPU, VkDevice& device, VmaAllocator& allocator, DeletionQueue& deletionQueue);
        VkSwapchainKHR swapchain; // from other articles

        // image format expected by the windowing system
        VkFormat swapchainImageFormat; 	
        
        //array of images from the swapchain
        std::vector<VkImage> swapchainImages;

        //array of image-views from the swapchain
        std::vector<VkImageView> swapchainImageViews;

        VkExtent2D actualExtent{};
        VkSurfaceKHR surface; // Vulkan window surface

        VkImageView depthImageView;
        AllocatedImage depthImage;

        //the format for the depth image
        VkFormat depthFormat;
        void InitSwapchain(SDL_Window* p_window);
    private: 
        VkPhysicalDevice* p_chosenGPU;
        VkDevice* p_device;
        VmaAllocator* p_allocator;
        DeletionQueue* p_deletionQueue;
    };
}
