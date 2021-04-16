#pragma once

#include "vk_types.h"
#include <vector>
#include <array>
#include <unordered_map>

namespace vkcomponent {
	

	class DescriptorAllocator {
	public:
		
		struct PoolSizes {
			std::vector<std::pair<VkDescriptorType,float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		void ResetPools();
		bool Allocate(VkDescriptorSet* p_set, VkDescriptorSetLayout layout);
		bool AllocateVariableSet(VkDescriptorSet* p_set, VkDescriptorSetLayout layout, const uint32_t& counts = 1);
		
		void Init(VkDevice newDevice);

		void CleanUp();

		VkDevice device;
	private:
		VkDescriptorPool GrabPool();

		VkDescriptorPool currentPool{VK_NULL_HANDLE};
		PoolSizes descriptorSizes;
		std::vector<VkDescriptorPool> usedPools;
		std::vector<VkDescriptorPool> freePools;
	};


	class DescriptorLayoutCache {
	public:
		void Init(VkDevice newDevice);
		void CleanUp();

		VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* p_info);

		struct DescriptorLayoutInfo {
			//good idea to turn this into a inlined array
			std::vector<VkDescriptorSetLayoutBinding> bindings;

			bool operator==(const DescriptorLayoutInfo& other) const;

			size_t hash() const;
		};

		

	private:

		struct DescriptorLayoutHash
		{

			std::size_t operator()(const DescriptorLayoutInfo& k) const
			{
				return k.hash();
			}
		};

		std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
		VkDevice device;
	};


	class DescriptorBuilder {
	public:

		static DescriptorBuilder Begin(DescriptorLayoutCache* p_layoutCache, DescriptorAllocator* p_allocator );

		DescriptorBuilder& BindBuffer(uint32_t binding, VkDescriptorBufferInfo* p_bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

		DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo* p_imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

		bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
		bool Build(VkDescriptorSet& set);
	private:
		
		std::vector<VkWriteDescriptorSet> writes;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		

		DescriptorLayoutCache* p_cache;
		DescriptorAllocator* p_alloc;
	};
}
