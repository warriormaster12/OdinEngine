#pragma once 

#include "vk_types.h"

//this includes functions that initialize parts of the Vulkan creation process.
// such as renderpasses, framebuffers etc.

class vk_functions
{
public:
    static void CreateFramebuffer(VkFramebuffer& frameBuffer, VkRenderPass& renderPass,std::vector <VkImageView> attachments, VkExtent2D& imageExtent, const uint32_t& imageCount = 1);
private:
};