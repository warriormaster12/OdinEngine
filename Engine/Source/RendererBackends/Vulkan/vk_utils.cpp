#include "vk_utils.h"
#include "vk_check.h"


void CreateBuffer(
    const VmaAllocator& allocator,
    AllocatedBuffer* outBuffer,
    CreateBufferInfo info
) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = info.allocSize;
    bufferInfo.usage = info.bufferUsage;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = info.memoryUsage;

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(
        allocator,
        &bufferInfo,
        &vmaAllocInfo,
        &outBuffer->buffer,
        &outBuffer->allocation,
        &outBuffer->allocationInfo
    ));
}
