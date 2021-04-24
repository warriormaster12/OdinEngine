#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vk_mem_alloc.h"


struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
	VkImageView defaultView;
	int mipLevels;
};



