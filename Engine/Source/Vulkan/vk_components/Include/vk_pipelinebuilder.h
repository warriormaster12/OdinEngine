#pragma once 

#include "vk_types.h"
#include "logger.h"

#include <vector>
#include <array>

namespace vkcomponent
{
    class PipelineBuilder {
    public:

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        VkPipelineVertexInputStateCreateInfo vertexInputInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkViewport viewport;
        VkRect2D scissor;
        std::array<float, 4> blendConstant;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipeline BuildPipeline(VkDevice& device, VkRenderPass& pass);
    };
}