#include "Include/vk_init.h"
#include <iostream>

VkCommandPoolCreateInfo vkinit::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
{
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;

    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;
    return info;
}

VkCommandBufferAllocateInfo vkinit::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/, VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;

    info.commandPool = pool;
    info.commandBufferCount = count;
    info.level = level;
    return info;
}

VkPipelineShaderStageCreateInfo vkinit::PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule) {

    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    //shader stage
    info.stage = stage;
    //module containing the code for this shader stage
    info.module = shaderModule;
    //the entry point of the shader
    info.pName = "main";
    return info;
}

VkPipelineVertexInputStateCreateInfo vkinit::VertexInputStateCreateInfo() {
		VkPipelineVertexInputStateCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.pNext = nullptr;

		//no vertex bindings or attributes
		info.vertexBindingDescriptionCount = 0;
		info.vertexAttributeDescriptionCount = 0;
		return info;
}

VkPipelineInputAssemblyStateCreateInfo vkinit::InputAssemblyCreateInfo(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    //we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}

VkPipelineRasterizationStateCreateInfo vkinit::RasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode /* VK_CULL_MODE_BACK_BIT*/)
{
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthClampEnable = VK_FALSE;
    //discards all primitives before the rasterization stage if enabled which we don't want
    info.rasterizerDiscardEnable = VK_FALSE;

    info.polygonMode = polygonMode;
    info.lineWidth = 1.0f;
    //select cull mode
    info.cullMode = cullMode;
    info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //no depth bias
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f; 
    info.depthBiasSlopeFactor = 0.0f; 

    return info;
}

VkPipelineMultisampleStateCreateInfo vkinit::MultisamplingStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    //multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendAttachmentState vkinit::ColorBlendAttachmentState() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor =  VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vkinit::PipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    //empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkFramebufferCreateInfo vkinit::FramebufferCreateInfo(VkRenderPass& renderPass, VkExtent2D& windowExtent)
{
    VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = nullptr;
    
	info.renderPass = renderPass;
	info.attachmentCount = 1;
	info.width = windowExtent.width;
	info.height = windowExtent.height;
	info.layers = 1;
    return info;
}

 VkFenceCreateInfo vkinit::FenceCreateInfo(VkFenceCreateFlags flags /*= 0*/)
 {
    VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = flags;
	info.pNext = nullptr;

    return info;
 }
    
VkSemaphoreCreateInfo vkinit::SemaphoreCreateInfo()
{
    VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;

    return info;
}

VkImageCreateInfo vkinit::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo vkinit::ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

VkPipelineDepthStencilStateCreateInfo vkinit::DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
{
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
    info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f; // Optional
    info.maxDepthBounds = 1.0f; // Optional
    info.stencilTestEnable = VK_FALSE;

    return info;
}

VkRenderPassBeginInfo vkinit::RenderpassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer)
{
	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.pNext = nullptr;

	info.renderPass = renderPass;
	info.renderArea.offset.x = 0;
	info.renderArea.offset.y = 0;
	info.renderArea.extent = windowExtent;
	info.clearValueCount = 1;
	info.pClearValues = nullptr;
	info.framebuffer = framebuffer;

	return info;
}

VkSubmitInfo vkinit::SubmitInfo(VkCommandBuffer* p_cmd)
{
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;

	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.pWaitDstStageMask = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = p_cmd;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;

	return info;
}

VkPresentInfoKHR vkinit::PresentInfo()
{
	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext = nullptr;

	info.swapchainCount = 0;
	info.pSwapchains = nullptr;
	info.pWaitSemaphores = nullptr;
	info.waitSemaphoreCount = 0;
	info.pImageIndices = nullptr;

	return info;
}

VkCommandBufferBeginInfo vkinit::CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/)
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

VkDescriptorSetLayoutBinding vkinit::DescriptorsetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
	VkDescriptorSetLayoutBinding setbind = {};
	setbind.binding = binding;
	setbind.descriptorCount = 1;
	setbind.descriptorType = type;
	setbind.pImmutableSamplers = nullptr;
	setbind.stageFlags = stageFlags;

	return setbind;
}

VkWriteDescriptorSet vkinit::WriteDescriptorBuffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo , uint32_t binding)
{
	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = bufferInfo;

	return write;
}

VkSamplerCreateInfo vkinit::SamplerCreateInfo(VkFilter filters, VkSamplerAddressMode samplerAdressMode /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/)
{
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = nullptr;

	info.magFilter = filters;
	info.minFilter = filters;
	info.addressModeU = samplerAdressMode;
	info.addressModeV = samplerAdressMode;
	info.addressModeW = samplerAdressMode;

	return info;
}
VkWriteDescriptorSet vkinit::WriteDescriptorImage(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* p_imageInfo, uint32_t binding)
{
	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = p_imageInfo;

	return write;
}

VkDescriptorSetLayoutCreateInfo vkinit::DescriptorLayoutInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.bindingCount = bindings.size();
	setinfo.flags = 0;
	setinfo.pNext = nullptr;
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pBindings = bindings.data();

    return setinfo;
}






