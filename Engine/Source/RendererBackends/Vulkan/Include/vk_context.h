#pragma once 

#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "vk_pipelinebuilder.h"
#include "vk_types.h"
#include "vk_utils.h"
#include "vk_meshhandler.h"
#include "function_queuer.h"

struct ShaderProgram 
{
    vkcomponent::ShaderPass pass;
};

struct ShaderDescriptions
{
    std::vector <LocationInfo> vertexLocations;

    bool colorBlending = true;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;

    bool depthTesting = false;
    VkCompareOp depthCompareType = VK_COMPARE_OP_EQUAL;
};

struct DescripotrSetLayoutBindingInfo
{
    VkDescriptorType descriptorType;
    VkShaderStageFlags shaderStageFlags;
    uint32_t binding;
    uint32_t descriptorCount;
};

struct DescriptorSetLayoutInfo
{
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};


namespace VulkanContext 
{
    void InitVulkan();

    void ResizeWindow();

    void UpdateDraw(float clearColor[4],std::function<void()>&& drawCalls);

    void CleanUpVulkan(FunctionQueuer* p_additionalDeletion);

    void CreateDefaultRenderpass();
    void CreateMainFramebuffer();

    void CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo);
    void CreateDescriptorSetLayout(const std::string& layoutName);
    void RemoveDescriptorSetLayout(const std::string& layoutName);

    void CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkRenderPass& renderPass = VK_NULL_HANDLE, const ShaderDescriptions* descriptions = nullptr);
    
    void CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding ,const bool& frameOverlap ,AllocatedBuffer& allocatedBuffer, const size_t& dataSize, const size_t& byteOffset);
    
    void CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter);
    
    void DestroySampler(const std::string& samplerName);

    void CreateDescriptorSetImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views,const VkFormat& imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer);

    void BindGraphicsPipeline(const std::string& shaderName);
    void BindDescriptorSet(const std::string& descriptorName, const std::string& shaderName, const uint32_t& set, const bool& frameOverlap,const bool& isDynamic, const size_t& dataSize);
    void BindIndexBuffer(AllocatedBuffer& indexBuffer);
    void BindVertexBuffer(AllocatedBuffer& vertexBuffer);
    void DrawIndexed(std::vector<std::uint32_t>& indices);

    void BeginRenderpass(const float clearColor[4], const VkRenderPass& renderPass = VK_NULL_HANDLE);
    void EndRenderpass();
}