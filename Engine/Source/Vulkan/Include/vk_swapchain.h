#pragma once 

#include "vk_types.h"
#include "vk_init.h"
#include "function_queuer.h"
#include <iostream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



class SwapChain
{
public:
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
    void InitSwapchain(GLFWwindow* p_window, FunctionQueuer& deletionQueue);
};

