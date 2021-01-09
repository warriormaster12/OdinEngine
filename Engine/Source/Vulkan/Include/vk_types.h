#pragma once
#include <vulkan/vulkan.h>
#include "../../third-party/Vma/vk_mem_alloc.h"

struct AllocatedBuffer {
    VkBuffer _buffer;
    VmaAllocation _allocation;
};
struct AllocatedImage {
    VkImage _image;
    VmaAllocation _allocation;
	VkImageView _defaultView;
	int mipLevels;
};

