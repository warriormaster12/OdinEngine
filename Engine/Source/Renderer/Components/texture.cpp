#include "Include/texture.h"
#include "asset_builder.h"

#include "vk_texture.h"
#include "vk_init.h"
#include "vk_device.h"
#include "renderer.h"


void Texture::CreateTexture(const std::string& filepath)
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        vkcomponent::LoadImageFromFile(filepath, image);

        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageViewCreateInfo imageInfo = vkinit::ImageViewCreateInfo(imageFormat, image.image, VK_IMAGE_ASPECT_COLOR_BIT);

        vkCreateImageView(VkDeviceManager::GetDevice(), &imageInfo, nullptr, &imageView);
    }
}

void Texture::DestroyTexture()
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        vkDestroyImageView(VkDeviceManager::GetDevice(), imageView, nullptr);
        vkDestroyImage(VkDeviceManager::GetDevice(), image.image, nullptr);
    }
}