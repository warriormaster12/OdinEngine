#pragma once 

#include <iostream>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>



#include "vk_commandbuffer.h"
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

struct VkPushConstantInfo
{
    uint32_t offset;
    uint32_t dataSize;
    VkShaderStageFlags shaderStage;
};

struct VkShaderDescriptions
{
    std::vector <VkLocationInfo> vertexLocations;

    bool colorBlending = true;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;

    bool depthTesting = false;
    VkCompareOp depthCompareType = VK_COMPARE_OP_EQUAL;

    std::unique_ptr<VkPushConstantInfo> p_pushConstant = nullptr;

    std::string renderPassName = "main pass";
};

struct VkFrameBufferAdditionalInfo
{
    std::string renderPass;
    uint32_t width;
    uint32_t height;
    bool resizable; 
    std::vector<AllocatedImage> images;
    std::vector<VkFormat> imageFormats  = {VK_FORMAT_R8G8B8A8_UNORM, {}};

    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkAttachmentReference> attachmentRefs = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
};

struct VkRenderPassInfo 
{
    std::vector<VkClearValue> clearValues;
    std::string frameBufferName;

    FunctionQueuer passQueue;
};




class VulkanContext 
{
public: 
    static void InitVulkan();

    static void ResizeWindow();

    static void UpdateDraw();

    static void CleanUpVulkan(FunctionQueuer* p_additionalDeletion);

    static void CreateFramebuffer(const std::string& bufferName, std::unique_ptr<VkFrameBufferAdditionalInfo> p_bufferInfo);
    

    static void CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo);
    static void CreateDescriptorSetLayout(const std::string& layoutName);
    static void RemoveDescriptorSetLayout(const std::string& layoutName);

    static void CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkShaderDescriptions& descriptions);
    
    static void CreateUniformBufferInfo(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& dataRange);

    static void CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding ,const bool& frameOverlap ,const std::string& bufferName, const size_t& byteOffset);
    
    static void CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter, const VkSamplerAddressMode& samplerAddressMode);
    
    static void DestroySampler(const std::string& samplerName);

    static void CreateDescriptorSetImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views,const VkFormat& imageFormat = VK_FORMAT_R8G8B8A8_SRGB);

    static void CreateDescriptorSetFrameBufferImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::string& bufferName);

    static void RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap);

    static void BindGraphicsPipeline(const std::string& shaderName);
    static void PushConstants(const VkShaderStageFlags& shaderStage, const uint32_t& offset, const uint32_t& dataSize, const void* data);
    static void BindDescriptorSet(const std::string& descriptorName, const uint32_t& set,const uint32_t& dynamicOffset, const bool& frameOverlap);
    static void BindIndexBuffer(AllocatedBuffer& indexBuffer);
    static void BindVertexBuffer(AllocatedBuffer& vertexBuffer);

    static void Draw(const uint32_t& vertices, const uint32_t& instanceCount, const uint32_t& firstVertex, const uint32_t& firstInstance);

    static void DrawIndexed(std::vector<uint32_t>& indices, const uint32_t& currentInstance);

    static void PrepareIndirectDraw(const uint32_t& MAX_COMMANDS);
    static void UploadIndirectDraw(const uint32_t& objectCount, const std::vector<uint32_t>& indexSize, const uint32_t& currentInstance);
    static void DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex);


    static VkRenderPass& GetRenderpass();

    static void AddDrawToRenderpassQueue(std::function<void()>&& drawCalls, const std::string& passName ="main pass");
    static void PrepareRenderpassForDraw(const float& clearValueCount, const float clearColor[4], const float& depth, const std::string& passName ="main pass", const std::string& frameBufferName ="main framebuffer");

    // template functions 
    template<typename T>
    static void UploadSingleBufferData(const std::string& bufferName, const T& data,const bool& frameOverlap, const size_t& byteOffset)
    {
        if(frameOverlap)
        {
            for(int i = 0; i < FRAME_OVERLAP; i++)
            {
                UploadSingleData(FindUnorderdMap(bufferName, VkCommandbufferManager::GetFrames(i).allocatedBuffer)->allocation, data, byteOffset);
            }
        }
        else
        {
            UploadSingleData(FindUnorderdMap(bufferName, allocatedBuffers)->allocation, data, byteOffset);
        }
        
        
    }
    template<typename T>
    static void UploadVectorBufferData(const std::string& bufferName, const std::vector<T>& data,const bool& frameOverlap)
    {
        if(frameOverlap)
        {
            for(int i = 0; i < FRAME_OVERLAP; i++)
            {
                UploadVectorData(FindUnorderdMap(bufferName, VkCommandbufferManager::GetFrames(i).allocatedBuffer)->allocation, data);
            }
            
        }
        else
        {
            UploadVectorData(FindUnorderdMap(bufferName, allocatedBuffers)->allocation, data);
        }
        
        
    }
private: 
    static inline std::unordered_map<std::string, AllocatedBuffer> allocatedBuffers;
};