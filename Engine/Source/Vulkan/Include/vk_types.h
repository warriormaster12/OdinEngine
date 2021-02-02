#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../../Third-Party/Vma/vk_mem_alloc.h"

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

