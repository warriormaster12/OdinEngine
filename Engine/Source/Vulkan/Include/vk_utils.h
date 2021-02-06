#pragma once

#include "vk_types.h"

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
void UploadSingleData(const VmaAllocator& allocator, const VmaAllocation& allocation, const T& data, size_t byteOffset = 0)
{
    UploadArrayData(allocator, allocation, &data, 1, byteOffset);
}
