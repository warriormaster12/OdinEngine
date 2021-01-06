#pragma once 

#include "../../Include/vk_types.h"
#include "../../Include/vk_renderer.h"


namespace vkcomponent 
{
    bool load_image_from_file(VulkanRenderer& renderer, const char* file, AllocatedImage& outImage);
}