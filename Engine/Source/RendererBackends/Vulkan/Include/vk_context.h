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
#include "unordered_finder.h"

struct ShaderProgram 
{
    vkcomponent::ShaderPass pass;
};

struct VkShaderDescriptions
{
    std::vector <VkLocationInfo> vertexLocations;

    bool colorBlending = true;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;

    bool depthTesting = false;
    VkCompareOp depthCompareType = VK_COMPARE_OP_EQUAL;
};




class VulkanContext 
{
public: 
    static void InitVulkan();

    static void ResizeWindow();

    static void UpdateDraw(float clearColor[4],std::function<void()>&& drawCalls);

    static void CleanUpVulkan(FunctionQueuer* p_additionalDeletion);

    static void CreateDefaultRenderpass();
    static void CreateMainFramebuffer();

    static void CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo);
    static void CreateDescriptorSetLayout(const std::string& layoutName);
    static void RemoveDescriptorSetLayout(const std::string& layoutName);

    static void CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkShaderDescriptions& descriptions, const VkRenderPass& renderPass = VK_NULL_HANDLE);
    
    static void CreateUniformBufferInfo(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& byteOffset);

    static void CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding ,const bool& frameOverlap ,const std::string& bufferName);
    
    static void CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter, const VkSamplerAddressMode& samplerAddressMode);
    
    static void DestroySampler(const std::string& samplerName);

    static void CreateDescriptorSetImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views,const VkFormat& imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

    static void RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap);

    static void BindGraphicsPipeline(const std::string& shaderName);
    static void BindDescriptorSet(const std::string& descriptorName, const uint32_t& set, const bool& frameOverlap,const bool& isDynamic, const size_t& dataSize);
    static void BindIndexBuffer(AllocatedBuffer& indexBuffer);
    static void BindVertexBuffer(AllocatedBuffer& vertexBuffer);
    static void DrawIndexed(std::vector<std::uint32_t>& indices, const uint32_t& currentInstance);

    static void PrepareIndirectDraw(const uint32_t& MAX_COMMANDS);
    static void UploadIndirectDraw(const uint32_t& objectCount, const std::vector<uint32_t>& indexSize, const uint32_t& currentInstance);
    static void DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex);

    static void BeginRenderpass(const float clearColor[4], const VkRenderPass& renderPass = VK_NULL_HANDLE);
    static void EndRenderpass();

    // template functions 
    template<typename T>
    static void UploadSingleBufferData(const std::string& bufferName, const T& data,const bool& frameOverlap)
    {
        if(frameOverlap)
        {
            UploadSingleData(FindUnorderdMap(bufferName, VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->allocation, data);
        }
        else
        {
            UploadSingleData(FindUnorderdMap(bufferName, allocatedBuffers)->allocation, data);
        }
        
        
    }
    template<typename T>
    static void UploadVectorBufferData(const std::string& bufferName, const std::vector<T>& data,const bool& frameOverlap)
    {
        if(frameOverlap)
        {
            UploadVectorData(FindUnorderdMap(bufferName, VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->allocation, data);
        }
        else
        {
            UploadVectorData(FindUnorderdMap(bufferName, allocatedBuffers)->allocation, data);
        }
        
        
    }
private: 
    inline static std::unordered_map<std::string, AllocatedBuffer> allocatedBuffers;
};