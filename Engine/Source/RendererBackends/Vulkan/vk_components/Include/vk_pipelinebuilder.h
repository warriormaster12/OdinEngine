#pragma once 

#include "vk_types.h"
#include "logger.h"

#include <vector>
#include <array>
#include <memory>
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
        VkPipeline BuildPipeline(VkRenderPass& pass);

        void SetShaders(ShaderEffect* effect);
    };


    class ComputePipelineBuilder {
    public:

        VkPipelineShaderStageCreateInfo  shaderStage;
        VkPipelineLayout pipelineLayout;
        VkPipeline BuildPipeline();

        void SetShaders(ShaderEffect* effect);
    };

    struct ShaderPass {
		ShaderEffect* effect {nullptr};
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout layout{ VK_NULL_HANDLE };

        void FlushPass();
	};

    //ShaderEffect* BuildEffect(VkDevice& device, std::vector<vkcomponent::ShaderModule>& shaders, std::vector<ShaderEffect::ReflectionOverrides> overrides={});
    ShaderEffect* BuildEffect(std::vector<vkcomponent::ShaderModule>& shaders, VkPipelineLayoutCreateInfo& info);
    ShaderPass* BuildShader(VkRenderPass renderPass,PipelineBuilder& builder, ShaderEffect* effect);
}