#pragma once 

#include "vk_types.h"
#include "logger.h"

#include <vector>
#include <array>
#include "vk_shaderhandler.h"


class VulkanRenderer;
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

        void SetShaders(ShaderEffect* effect);
    };

    struct ShaderPass {
		ShaderEffect* effect{ nullptr };
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout layout{ VK_NULL_HANDLE };
	};

    ShaderEffect* BuildEffect(VkDevice& device, std::vector<vkcomponent::ShaderModule>& shaders, std::vector<ShaderEffect::ReflectionOverrides> overrides={});
    ShaderPass* BuildShader(VkDevice& device, VkRenderPass renderPass,PipelineBuilder& builder, ShaderEffect* effect);
}