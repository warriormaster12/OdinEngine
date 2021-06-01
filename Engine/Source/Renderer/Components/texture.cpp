#include "Include/texture.h"
#include "asset_builder.h"

#include "vk_texture.h"
#include "vk_init.h"
#include "vk_device.h"
#include "renderer.h"
#include <vulkan/vulkan_core.h>


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

void Texture::CreateCubeMapTexture(const std::vector<std::string>& filepaths)
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        vkcomponent::LoadImageFromFiles(filepaths, image);

        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageViewCreateInfo imageInfo = vkinit::ImageViewCreateInfo(imageFormat, image.image, VK_IMAGE_ASPECT_COLOR_BIT);
        imageInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        imageInfo.subresourceRange.layerCount = 6;

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