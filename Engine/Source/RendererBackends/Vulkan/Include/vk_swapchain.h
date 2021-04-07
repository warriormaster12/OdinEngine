#pragma once 

#include "vk_types.h"
#include <vector>
#include <iostream>


class VkSwapChainManager
{
public: 
    
    static void InitSwapchain();
    static void DeleteSwapchain();
    static VkExtent2D& GetSwapchainExtent() { return swapExtent; }
    static std::vector<VkImageView> GetSwapchainImageViews() { return swapchainImageViews;}
    static VkImageView GetSwapchainDepthView() { return depthImageView;}
    static VkFormat& GetSwapchainImageFormat() { return swapchainImageFormat; }
    static VkFormat& GetSwapchainDepthFormat() { return depthFormat; }
    static VkSwapchainKHR& GetSwapchain() { return swapchain; }
private: 
    inline static VkSwapchainKHR swapchain; // from other articles

    // image format expected by the windowing system
    inline static VkFormat swapchainImageFormat; 	
    
    //array of images from the swapchain
    inline static std::vector<VkImage> swapchainImages;

    //array of image-views from the swapchain
    inline static std::vector<VkImageView> swapchainImageViews;

    inline static VkExtent2D swapExtent{};

    inline static VkImageView depthImageView;
    inline static AllocatedImage depthImage;

    //the format for the depth image
    inline static VkFormat depthFormat;
};
