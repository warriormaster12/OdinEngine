#pragma once 

#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "vk_pipelinebuilder.h"
#include "vk_types.h"

struct ShaderProgram 
{
    vkcomponent::ShaderPass pass;
};

struct DescriptorSetLayout
{
    std::vector<VkDescriptorSetLayout> layouts;
};

struct DescripotrSetLayoutBindingInfo
{
    VkDescriptorType descriptorType;
    VkShaderStageFlags shaderStageFlags;
    uint32_t binding;
    uint32_t descriptorCount;
};


namespace VulkanContext 
{
    void InitVulkan();

    void ResizeWindow();

    void UpdateDraw(float clearColor[4],std::function<void()>&& drawCalls);

    void CleanUpVulkan();

    void CreateDefaultRenderpass();
    void CreateMainFramebuffer();

    void CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo);
    void CreateDescriptorSetLayout(const std::string& layoutName);
    void RemoveDescriptorSetLayout(const std::string& layoutName);

    void CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::string& layoutName, const VkRenderPass& renderPass = VK_NULL_HANDLE);
    
    void CreateDescriptorSet(const std::string& descriptorName, AllocatedBuffer& allocatedBuffer, const size_t& dataSize, size_t byteOffset = 0);
    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer);

    void BindGraphicsPipeline(const std::string& shaderName);
    void BindDescriptorSet(const std::string& descriptorName, const std::string& shaderName);
    void Draw();

    void BeginRenderpass(const float clearColor[4], const VkRenderPass& renderPass = VK_NULL_HANDLE);
    void EndRenderpass();
}