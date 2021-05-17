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
    static inline VkSwapchainKHR swapchain; // from other articles

    // image format expected by the windowing system
    static inline VkFormat swapchainImageFormat; 	
    
    //array of images from the swapchain
    static inline std::vector<VkImage> swapchainImages;

    //array of image-views from the swapchain
    static inline std::vector<VkImageView> swapchainImageViews;

    static inline VkExtent2D swapExtent{};

    static inline VkImageView depthImageView;
    static inline AllocatedImage depthImage;

    //the format for the depth image
    static inline VkFormat depthFormat;
};
