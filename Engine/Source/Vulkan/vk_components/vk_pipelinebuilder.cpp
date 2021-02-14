#include "Include/vk_pipelinebuilder.h"
#include <iostream>
#include <array>


namespace vkcomponent
{
    VkPipeline PipelineBuilder::BuildPipeline(VkDevice& device, VkRenderPass& pass)
    {
        //make viewport state from our stored viewport and scissor.
            //at the moment we wont support multiple viewports or scissors
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;

        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        //setup dummy color blending. We arent using transparent objects yet
        //the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.pNext = nullptr;

        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = blendConstant[0];
        colorBlending.blendConstants[1] = blendConstant[1];
        colorBlending.blendConstants[2] = blendConstant[2];
        colorBlending.blendConstants[3] = blendConstant[3];

        //build the actual pipeline
        //we now use all of the info structs we have been writing into into this one to create the pipeline
        std::array <VkDynamicState, 2> dStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}; 
        VkPipelineDynamicStateCreateInfo dStateInfo = {};
        dStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dStateInfo.dynamicStateCount = dStates.size();
        dStateInfo.pDynamicStates = dStates.data();
        
        
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;

        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dStateInfo;

        //its easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
        VkPipeline newPipeline;
        if (vkCreateGraphicsPipelines(
            device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
            ENGINE_CORE_ERROR("failed to create pipline\n");
            return VK_NULL_HANDLE; // failed to create graphics pipeline
        }
        else
        {
            return newPipeline;
        }
    }
}