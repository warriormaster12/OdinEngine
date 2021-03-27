#include "Include/vk_functions.h"
#include "vk_check.h"
#include "vk_device.h"
#include "vk_init.h"


void vk_functions::CreateFramebuffer(VkFramebuffer& frameBuffer, VkRenderPass& renderPass,std::vector <VkImageView> attachments, VkExtent2D& imageExtent, const uint32_t& imageCount /*= 1*/)
{
    VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(renderPass, imageExtent);
    for (int i = 0; i < imageCount; i++)
    {
        fbInfo.attachmentCount = attachments.size();
        fbInfo.pAttachments = attachments.data();
        vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbInfo, nullptr, &frameBuffer);
    }
}
