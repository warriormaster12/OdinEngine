#pragma once 

#include "vk_types.h"
#include "vk_renderer.h"


namespace vkcomponent 
{
    /**
     * Loads and allocates a VkImage from a file containing raw pixel data.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param renderer The VulkanRenderer instance
     * @param p_filename The name of the file
     * @param outImage A pointer to write the result to
     * @return True, if the load was successful, false otherwise
     */
    bool LoadImageFromFile(VulkanRenderer& renderer, const char* p_filename, AllocatedImage* outImage);

    /**
     * Loads and allocates a VkImage from a binary file.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param renderer The VulkanRenderer instance
     * @param p_filename The name of the file
     * @param outImage A pointer to write the result to
     * @return True, if the load was successful, false otherwise
     */
    bool LoadImageFromAsset(VulkanRenderer& renderer, const char* p_filename, AllocatedImage* outImage);

    /**
     * Loads and allocates a VkImage consisting of a 1x1 black and transparent pixel.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param renderer The VulkanRenderer instance
     * @param outImage A pointer to write the result to
     */
    void LoadEmpty(VulkanRenderer& renderer, AllocatedImage* outImage);
}