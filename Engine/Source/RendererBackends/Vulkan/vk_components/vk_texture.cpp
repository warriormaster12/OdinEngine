#include "Include/vk_texture.h"
#include "vk_init.h"
#include "vk_utils.h"
#include "vk_device.h"
#include "vk_commandbuffer.h"
#include "logger.h"
#include "vk_check.h"
#include "stb_image.h"
#include "texture_asset.h"
#include "asset_loader.h"

namespace {
    void UploadImage(
        int texWidth,
        int texHeight,
        VkFormat imageFormat,
        AllocatedBuffer& stagingBuffer,
        AllocatedImage& outImage
    ) {
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(texWidth);
        imageExtent.height = static_cast<uint32_t>(texHeight);
        imageExtent.depth = 1;

        VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        //allocate and create the image
        vmaCreateImage(VkDeviceManager::GetAllocator(), &dimg_info, &dimg_allocinfo, &outImage.image, &outImage.allocation, nullptr);

        //transition image to transfer-receiver	
        VkCommandbufferManager::ImmediateSubmit([&](VkCommandBuffer cmd) {
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
            imageBarrier_toTransfer.image = outImage.image;
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
            vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, outImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageBarrier_toReadable.subresourceRange = range;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
        });


		// TODO: Mip levels
        outImage.mipLevels = 1;
    }

	void UploadCubeMapImage(
        int texWidth,
        int texHeight,
		std::vector<size_t>& offsets,
        VkFormat imageFormat,
        AllocatedBuffer& stagingBuffer,
        AllocatedImage& outImage
    ) {
        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(texWidth);
        imageExtent.height = static_cast<uint32_t>(texHeight);
        imageExtent.depth = 1;

        VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);
		dimg_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		dimg_info.sharingMode =  VK_SHARING_MODE_EXCLUSIVE;
		dimg_info.arrayLayers = 6;
		dimg_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        //allocate and create the image
        vmaCreateImage(VkDeviceManager::GetAllocator(), &dimg_info, &dimg_allocinfo, &outImage.image, &outImage.allocation, nullptr);

        //transition image to transfer-receiver	
		std::vector<VkBufferImageCopy> bufferCopyRegions;
        VkCommandbufferManager::ImmediateSubmit([&](VkCommandBuffer cmd) {
			for(int i = 0; i < offsets.size(); i++)
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = 0;
				bufferCopyRegion.imageSubresource.baseArrayLayer = i;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = texWidth;
				bufferCopyRegion.imageExtent.height = texHeight;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offsets[i];
				bufferCopyRegions.push_back(bufferCopyRegion);

			}
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = 1;
            range.baseArrayLayer = 0;
            range.layerCount = offsets.size();

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.image = outImage.image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            //barrier the image into the transfer-receive layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

            //copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, outImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;


            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
        });


		// TODO: Mip levels
        outImage.mipLevels = 1;
    }
}


bool vkcomponent::LoadImageFromFile(const std::string& filename, AllocatedImage& outImage, VkFormat imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{
	if (filename == "")
	{
		ENGINE_CORE_WARN("Failed to load image file: Filename is null");
		LoadEmpty(outImage);
		return false;
	}

	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		ENGINE_CORE_WARN("Failed to load image file: Could not load image '{0}'", filename);
		LoadEmpty(outImage);
		return false;
	}
	VkDeviceSize imageSize = static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight) * 4 * sizeof(stbi_uc);

    //allocate temporary buffer for holding texture data to upload
	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
	}

	UploadArrayData(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, pixels, imageSize);
    //we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
	stbi_image_free(pixels);

    UploadImage(texWidth, texHeight, imageFormat, stagingBuffer, outImage);

	vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);

	ENGINE_CORE_INFO("Texture loaded succesfully {0}", filename);

    return true;
}

bool vkcomponent::LoadImageFromFiles(const std::vector<std::string>& filenames, AllocatedImage& outImage, VkFormat imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{

	int texWidth, texHeight, texChannels;

	std::vector<stbi_uc*> pixels;
	pixels.resize(filenames.size());
	for(int i = 0; i < filenames.size(); i++)
	{
		pixels[i] = stbi_load(filenames[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels[i]) {
			ENGINE_CORE_WARN("Failed to load image file: Could not load image '{0}'", filenames[i]);
			LoadEmpty(outImage);
			return false;
		}
	}
	
	VkDeviceSize imageSize = static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight) * 4 * sizeof(stbi_uc);

    //allocate temporary buffer for holding texture data to upload
	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize * filenames.size();
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
	}
	//for now this function is commented because I don't know how to pack data properly yet into it.
	//UploadArrayData(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, pixels.data(),imageSize);

	char* pData = nullptr;
	size_t byteOffset = 0;
	std::vector<size_t> byteOffsets;
	byteOffsets.resize(filenames.size());
    vmaMapMemory(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, (void**)&pData);
    //Forward pointer
	for(int i = 0; i < pixels.size(); i++)
	{
		pData += byteOffset;
		memcpy(pData, pixels[i], imageSize * sizeof(stbi_uc));
		byteOffset = imageSize;	
	}
	for(int i = 0; i < byteOffsets.size(); i++)
	{
		if(i == 0)
		{
			byteOffsets[i] = 0;
		}
		else {
			byteOffsets[i] += imageSize + byteOffsets[i-1];
		}
	}
	
    vmaUnmapMemory(VkDeviceManager::GetAllocator(), stagingBuffer.allocation);

    //we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
	for(auto& currentPixeData : pixels)
	{
		stbi_image_free(currentPixeData);
	}

    UploadCubeMapImage(texWidth, texHeight,byteOffsets,imageFormat, stagingBuffer, outImage);

	vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	for(auto& currentFile : filenames)
	{
		ENGINE_CORE_INFO("Texture {0} loaded succesfully for a target cube map", currentFile);
	}

    return true;
}


bool vkcomponent::LoadImageFromAsset(const std::string& filename, AllocatedImage& outImage)
{
	if (filename == "")
	{
		ENGINE_CORE_WARN("Failed to load image asset: Filename is null");
		LoadEmpty(outImage);
		return false;
	}

	assets::AssetFile file;
	if (!assets::load_binaryfile(filename.c_str(), file)) {
		ENGINE_CORE_WARN("Failed to load image asset: Could not load binary file '{0}'", filename);
		LoadEmpty(outImage);
		return false;
	}
	
	assets::TextureInfo textureInfo = ReadTextureInfo(&file);
	
	VkDeviceSize imageSize = textureInfo.textureSize;
	VkFormat imageFormat;
	switch (textureInfo.textureFormat) {
	case assets::TextureFormat::RGBA8:
		imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case assets::TextureFormat::UNORM8:
		imageFormat =  VK_FORMAT_R8G8B8A8_UNORM;
		break;
	default:
		ENGINE_CORE_WARN("Failed to load image asset: Unknown texture format '{0}', {1}", filename, textureInfo.textureFormat);
		LoadEmpty(outImage);
		return false;
	}

    AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
	}

	// Note: vmaMapMemory allocates memory for data and vmaUnmapMemory deallocates memory
	void* data;
	vmaMapMemory(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, &data);
	assets::UnpackTexture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), (char*)data);	
	vmaUnmapMemory(VkDeviceManager::GetAllocator(), stagingBuffer.allocation);	

	UploadImage(textureInfo.pixelsize[0], textureInfo.pixelsize[1], imageFormat, stagingBuffer, outImage);

	vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	
	return true;
}
void vkcomponent::LoadEmpty(AllocatedImage& outImage, VkFormat imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{
	const uint64_t texWidth = 1;
	const uint64_t texHeight = 1;

    float pixelData[] = {0,0,0,0};
	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(char);
	

    //allocate temporary buffer for holding texture data to upload
	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = imageSize;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
	}

    // copy pixel data to buffer
	UploadArrayData(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, pixelData, 4);
    UploadImage(texWidth, texHeight, imageFormat, stagingBuffer, outImage);

	vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
}