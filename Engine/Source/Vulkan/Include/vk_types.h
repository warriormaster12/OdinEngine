#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../third-party/Vma/vk_mem_alloc.h"

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};
struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
	VkImageView defaultView;
	int mipLevels;
};

