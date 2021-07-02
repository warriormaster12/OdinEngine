#include "Include/texture.h"
#include "asset_builder.h"

#include "logger.h"
#include "vk_texture.h"
#include "vk_init.h"
#include "vk_device.h"
#include "renderer.h"

#include "unordered_finder.h"
#include <unordered_map>



static AllocatedImage oldImage;
void Texture::CreateTexture(const std::string& filepath, const ColorFormat& imageFormat)
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        if(filepath != "")
        {
            if(image.image == VK_NULL_HANDLE)
            {
                vkcomponent::LoadImageFromFile(filepath, image, (VkFormat)imageFormat);

                VkImageViewCreateInfo imageInfo = vkinit::ImageViewCreateInfo((VkFormat)imageFormat, image.image, VK_IMAGE_ASPECT_COLOR_BIT);
                vkCreateImageView(VkDeviceManager::GetDevice(), &imageInfo, nullptr, &image.defaultView);
            }
            else {
                //we store old loaded image first so that we can safely delete it after loading a new one
                oldImage = image;

                vkcomponent::LoadImageFromFile(filepath, image, (VkFormat)imageFormat);
                VkImageViewCreateInfo imageInfo = vkinit::ImageViewCreateInfo((VkFormat)imageFormat, image.image, VK_IMAGE_ASPECT_COLOR_BIT);
                vkCreateImageView(VkDeviceManager::GetDevice(), &imageInfo, nullptr, &image.defaultView);

                vkDestroyImageView(VkDeviceManager::GetDevice(), oldImage.defaultView, nullptr);
                vmaDestroyImage(VkDeviceManager::GetAllocator(), oldImage.image, oldImage.allocation);

            }  
        }      
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

        vkCreateImageView(VkDeviceManager::GetDevice(), &imageInfo, nullptr, &image.defaultView);
    }
}

void Texture::DestroyTexture()
{
    if(Renderer::GetActiveAPI() == AvailableBackends::Vulkan)
    {
        if(image.image != VK_NULL_HANDLE)
        {
            vkDestroyImageView(VkDeviceManager::GetDevice(), image.defaultView, nullptr);
            vmaDestroyImage(VkDeviceManager::GetAllocator(), image.image, image.allocation);
        }
    }
}