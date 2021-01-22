#pragma once 

#include "../../Include/vk_types.h"
#include "../../Include/vk_renderer.h"


namespace vkcomponent 
{
    bool load_image_from_file(VulkanRenderer& renderer, const char* file, AllocatedImage& outImage);
    bool load_image_from_asset(VulkanRenderer& renderer, const char* filename, AllocatedImage& outImage);
    bool load_empty(VulkanRenderer& renderer, AllocatedImage& outImage);

    AllocatedImage upload_image(int texWidth, int texHeight, VkFormat image_format, VulkanRenderer& engine, AllocatedBuffer& stagingBuffer);
}