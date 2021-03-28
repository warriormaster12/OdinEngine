#pragma once 

#include "vk_types.h"

//this includes functions that initialize parts of the Vulkan creation process.
// such as renderpasses, framebuffers etc.

class vk_functions
{
public:
    static void CreateFramebuffer(VkFramebuffer& frameBuffer, VkRenderPass& renderPass,std::vector <VkImageView> attachments, VkExtent2D& imageExtent);
    static void DestroyFramebuffer(VkFramebuffer& frambuffer);
private:
};