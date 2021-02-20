#include "vk_textures.h"
#include "vk_init.h"
#include "vk_utils.h"
#include "vk_init.h"

#include "stb_image.h"
#include "texture_asset.h"
#include "asset_loader.h"
#include "logger.h"


namespace {
    void UploadImage(
        int texWidth,
        int texHeight,
        VkFormat image_format,
        VulkanRenderer& renderer,
        AllocatedBuffer& stagingBuffer,
        AllocatedImage* outImage
    ) {
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(texWidth);
        imageExtent.height = static_cast<uint32_t>(texHeight);
        imageExtent.depth = 1;

        VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        //allocate and create the image
        vmaCreateImage(renderer.GetAllocator(), &dimg_info, &dimg_allocinfo, &outImage->image, &outImage->allocation, nullptr);

        //transition image to transfer-receiver	
        renderer.ImmediateSubmit([&](VkCommandBuffer cmd) {
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = 1;
            range.baseArrayLayer = 0;
            range.layerCount = 1;

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.image = outImage->image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            //barrier the image into the transfer-receive layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

            VkBufferImageCopy copyRegion = {};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = imageExtent;

            //copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, outImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
        });


		// TODO: Mip levels
        outImage->mipLevels = 1;
    }
}


bool vkcomponent::LoadImageFromFile(VulkanRenderer& renderer, const char* p_filename, AllocatedImage* outImage)
{
	if (p_filename == nullptr)
	{
		ENGINE_CORE_WARN("Failed to load image file: Filename is null");
		LoadEmpty(renderer, outImage);
		return false;
	}

	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(p_filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		ENGINE_CORE_WARN("Failed to load image file: Could not load image '{0}'", p_filename);
		LoadEmpty(renderer, outImage);
		return false;
	}
	VkDeviceSize imageSize = static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight) * sizeof(stbi_uc);

    //the format R8G8B8A8 matchs exactly with the pixels loaded from stb_image lib
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    //allocate temporary buffer for holding texture data to upload
	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(renderer.GetAllocator(), &stagingBuffer, info);
	}

	UploadArrayData(renderer.GetAllocator(), stagingBuffer.allocation, pixels, imageSize);
    //we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
	stbi_image_free(pixels);

    UploadImage(texWidth, texHeight, image_format, renderer, stagingBuffer, outImage);

	vmaDestroyBuffer(renderer.GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);

	ENGINE_CORE_INFO("Texture loaded succesfully {0}", p_filename);

    return true;
}


bool vkcomponent::LoadImageFromAsset(VulkanRenderer& renderer, const char* p_filename, AllocatedImage* outImage)
{
	if (p_filename == nullptr)
	{
		ENGINE_CORE_WARN("Failed to load image asset: Filename is null");
		LoadEmpty(renderer, outImage);
		return false;
	}

	assets::AssetFile file;
	if (!assets::load_binaryfile(p_filename, file)) {
		ENGINE_CORE_WARN("Failed to load image asset: Could not load binary file '{0}'", p_filename);
		LoadEmpty(renderer, outImage);
		return false;
	}
	
	assets::TextureInfo textureInfo = read_texture_info(&file);
	
	VkDeviceSize imageSize = textureInfo.textureSize;
	VkFormat image_format;
	switch (textureInfo.textureFormat) {
	case assets::TextureFormat::RGBA8:
		image_format = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	default:
		ENGINE_CORE_WARN("Failed to load image asset: Unknown texture format '{0}', {1}", p_filename, textureInfo.textureFormat);
		LoadEmpty(renderer, outImage);
		return false;
	}

    AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(renderer.GetAllocator(), &stagingBuffer, info);
	}

	// Note: vmaMapMemory allocates memory for data and vmaUnmapMemory deallocates memory
	void* data;
	vmaMapMemory(renderer.GetAllocator(), stagingBuffer.allocation, &data);
	assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), (char*)data);	
	vmaUnmapMemory(renderer.GetAllocator(), stagingBuffer.allocation);	

	UploadImage(textureInfo.pixelsize[0], textureInfo.pixelsize[1], image_format, renderer, stagingBuffer, outImage);

	vmaDestroyBuffer(renderer.GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	
	return true;
}
void vkcomponent::LoadEmpty(VulkanRenderer& renderer, AllocatedImage* outImage)
{
	const uint64_t texWidth = 1;
	const uint64_t texHeight = 1;

    char pixelData[] = {0,0,0,0};
	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(char);

    //the format R8G8B8A8 matchs exactly with the pixels loaded from stb_image lib
	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

    //allocate temporary buffer for holding texture data to upload
	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(renderer.GetAllocator(), &stagingBuffer, info);
	}

    // copy pixel data to buffer
	UploadArrayData(renderer.GetAllocator(), stagingBuffer.allocation, pixelData, 4);
    UploadImage(texWidth, texHeight, image_format, renderer, stagingBuffer, outImage);

	vmaDestroyBuffer(renderer.GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
}

