#pragma once

#include "vk_types.h"

namespace vkinit {
    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();
    
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology);

    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT);

    VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

    VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass& renderPass, VkExtent2D& windowExtent);

    VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);

    VkSemaphoreCreateInfo SemaphoreCreateInfo();

    VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

    VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

    VkRenderPassBeginInfo RenderpassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

    VkSubmitInfo SubmitInfo(VkCommandBuffer* p_cmd);

    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    VkPresentInfoKHR PresentInfo();

    VkDescriptorSetLayoutBinding DescriptorsetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount = 1);

    VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* p_bufferInfo , uint32_t binding);

    VkSamplerCreateInfo SamplerCreateInfo(VkFilter filters, VkSamplerAddressMode samplerAdressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    
    VkWriteDescriptorSet WriteDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* p_imageInfo, uint32_t binding, uint32_t descriptorCount = 1);

    VkDescriptorSetLayoutCreateInfo DescriptorLayoutInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings, const void* bindingFlags = nullptr);
    
    VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorLayoutBindingFlagsInfo(const std::vector<VkDescriptorBindingFlagsEXT> flags);

    VkBool32 FormatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);
}

