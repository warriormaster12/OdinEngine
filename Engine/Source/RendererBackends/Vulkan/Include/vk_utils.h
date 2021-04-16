#pragma once

#include "vk_types.h"
#include "vk_device.h"

#include <cstring>

struct CreateBufferInfo {
    size_t allocSize;
    VkBufferUsageFlags bufferUsage;
    VmaMemoryUsage memoryUsage;
};

void CreateBuffer(const VmaAllocator& allocator, AllocatedBuffer* outBuffer, CreateBufferInfo info);

template<typename T>
void UploadArrayData(const VmaAllocator& allocator, const VmaAllocation& allocation, T* data, size_t len, size_t byteOffset = 0)
{
    // Note: pData is not initialized because vmaMapMemory(...) allocates memory for pData. Similarly,
    // vmaUnmapMemory deallocates the memory.
    char* pData;
    vmaMapMemory(allocator, allocation, (void**)&pData);
    // Forward pointer
    pData += byteOffset;
    memcpy(pData, data, len * sizeof(T));
    vmaUnmapMemory(allocator, allocation);
}

template<typename T>
void UploadVectorData(const VmaAllocator& allocator, const VmaAllocation& allocation, const std::vector<T>& data, size_t byteOffset = 0)
{
    UploadArrayData(allocator, allocation, data.data(), data.size(), byteOffset);
}

template<typename T>
void UploadSingleData(const VmaAllocation& allocation, const T& data, size_t byteOffset = 0)
{
    UploadArrayData(VkDeviceManager::GetAllocator(), allocation, &data, 1, byteOffset);
}

static const VkDescriptorBufferInfo& CreateDescriptorBuffer(AllocatedBuffer& inputBuffer, const size_t& dataSize, const VkBufferUsageFlags& bufferUsage,const size_t& dataOffset = 0)
{
    CreateBufferInfo info;
    info.allocSize = dataSize;
    info.bufferUsage = bufferUsage;
    info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    CreateBuffer(VkDeviceManager::GetAllocator(), &inputBuffer, info);

    VkDescriptorBufferInfo* descriptorBufferInfo = new VkDescriptorBufferInfo;
    descriptorBufferInfo->buffer = inputBuffer.buffer;
    descriptorBufferInfo->offset = dataOffset;
    descriptorBufferInfo->range = dataSize;

    return *descriptorBufferInfo;

    delete descriptorBufferInfo;
}

static size_t PadUniformBufferSize(size_t originalSize)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = VkDeviceManager::GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

