#pragma once 

#include "../../Include/vk_types.h"
#include "../../Include/vk_renderer.h"


namespace vkcomponent 
{
    bool LoadImageFromFile(VulkanRenderer& renderer, const char* p_file, AllocatedImage* outImage);
    bool LoadImageFromAsset(VulkanRenderer& renderer, const char* p_filename, AllocatedImage* outImage);
    bool LoadEmpty(VulkanRenderer& renderer, AllocatedImage* outImage);
}