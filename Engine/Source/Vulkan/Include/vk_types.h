#pragma once
#include <vulkan/vulkan.h>
#include "../../third-party/Vma/vk_mem_alloc.h"

struct AllocatedBuffer {
    VkBuffer _buffer;
    VmaAllocation _allocation;
};
