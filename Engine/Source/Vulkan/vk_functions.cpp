#include "Include/vk_functions.h"
#include "vk_check.h"
#include "vk_device.h"
#include "vk_init.h"


void vk_functions::CreateFramebuffer(VkFramebuffer& frameBuffer, VkRenderPass& renderPass,std::vector <VkImageView> attachments, VkExtent2D& imageExtent)
{
    VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(renderPass, imageExtent);
    fbInfo.attachmentCount = attachments.size();
    fbInfo.pAttachments = attachments.data();
    VK_CHECK(vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbInfo, nullptr, &frameBuffer));
}

void vk_functions::DestroyFramebuffer(VkFramebuffer& frambuffer)
{
    vkDestroyFramebuffer(VkDeviceManager::GetDevice(), frambuffer, nullptr);
}