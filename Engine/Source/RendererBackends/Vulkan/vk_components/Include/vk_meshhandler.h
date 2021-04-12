#pragma once

#include "vk_types.h"
#include "logger.h"
#include "vk_commandbuffer.h"
#include "vk_device.h"
#include "vk_utils.h"
#include <vector>
#include <iostream>




struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct LocationInfo
{
	VkFormat format;
	uint32_t offset;
};

struct VkVertex
{
	static VertexInputDescription GetVertexDescription(const std::vector<LocationInfo>& locations);
};

struct VkMesh
{
	template<typename T>
    static void UploadMeshData(std::vector<T> dataArray, AllocatedBuffer& buffer, const VkBufferUsageFlags& bufferUsageType)
	{
		const size_t BUFFER_SIZE = dataArray.size() * sizeof(dataArray[0]);

		AllocatedBuffer stagingBuffer;
		{
			CreateBufferInfo info;
			info.allocSize = BUFFER_SIZE;
			info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
			CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
		}

		//copy data
		UploadVectorData(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, dataArray);

		{
			CreateBufferInfo info;
			info.allocSize = BUFFER_SIZE;
			info.bufferUsage = bufferUsageType | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			CreateBuffer(VkDeviceManager::GetAllocator(), &buffer, info);
		}


		VkCommandbufferManager::ImmediateSubmit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy;
			copy.dstOffset = 0;
			copy.srcOffset = 0;
			copy.size = BUFFER_SIZE;
			vkCmdCopyBuffer(cmd, stagingBuffer.buffer, buffer.buffer, 1, & copy);
		});

		vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
	}

	static void DestroyBuffer(AllocatedBuffer& buffer)
	{
		vmaDestroyBuffer(VkDeviceManager::GetAllocator(), buffer.buffer, buffer.allocation);
	}
};