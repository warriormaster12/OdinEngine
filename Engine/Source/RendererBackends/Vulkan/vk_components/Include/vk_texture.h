#pragma once 

#include "vk_types.h"

#include <string>
#include <vector>

namespace vkcomponent 
{
    /**
     * Loads and allocates a VkImage from a file containing raw pixel data.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param filename The name of the file
     * @param outImage A pointer to write the result to
     * @return True, if the load was successful, false otherwise
     */
    bool LoadImageFromFile(const std::string& filename, AllocatedImage& outImage, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

    /**
     * Loads and allocates a VkImage from multiple files containing raw pixel data.
     *
     * Useful for example for cubemap textures
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param filenames names of files
     * @param outImage A pointer to write the result to
     * @return True, if the load was successful, false otherwise
     */

    bool LoadImageFromFiles(const std::vector<std::string>& filenames, AllocatedImage& outImage, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

    /**
     * Loads and allocates a VkImage from a binary file.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param filename The name of the file
     * @param outImage A pointer to write the result to
     * @return True, if the load was successful, false otherwise
     */
    bool LoadImageFromAsset(const std::string& filename, AllocatedImage& outImage);

    /**
     * Loads and allocates a VkImage consisting of a 1x1 black and transparent pixel.
     *
     * You will need to destroy the allocated image using VmaDestroyImage.
     *
     * @param outImage A pointer to write the result to
     */
    void LoadEmpty(AllocatedImage& outImage, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
}