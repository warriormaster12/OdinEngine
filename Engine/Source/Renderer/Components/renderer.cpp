#include "Include/renderer.h"
#include "logger.h"



AvailableBackends currentBackend;

AvailableBackends Renderer::GetActiveAPI()
{
    return currentBackend;
}
void Renderer::InitRenderer(AvailableBackends selectBackend)
{
    currentBackend = selectBackend;
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::InitVulkan();
    }
}

void Renderer::CreateFramebuffer(ObjectType type)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        if(type == ObjectType::Main)
        {
            VulkanContext::CreateMainFramebuffer();
        }
        else
        {
            ENGINE_CORE_ERROR("offscreen framebuffers aren't currently supported");   
        }
    }   
}

void Renderer::CreateRenderPass(ObjectType type)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        if(type == ObjectType::Main)
        {
            VulkanContext::CreateDefaultRenderpass();
        }
        else
        {
            ENGINE_CORE_ERROR("offscreen renderPasses aren't currently supported");   
        }
    } 
}
void Renderer::CreateShaderUniformLayoutBinding(const UniformType& uniformType, const ShaderStageFlags& shaderStage,const uint32_t& binding, const uint32_t& writeCount /*= 1*/)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        DescripotrSetLayoutBindingInfo info = {};
        info.descriptorType = (VkDescriptorType)uniformType;
        info.shaderStageFlags = shaderStage;
        info.binding = binding;
        info.descriptorCount = writeCount;
        VulkanContext::CreateDescriptorSetLayoutBinding(info);
    }
}

void Renderer::CreateShaderUniformLayout(const std::string& layoutName)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CreateDescriptorSetLayout(layoutName);
    }
}

void Renderer::RemoveShaderUniformLayout(const std::string& layoutName)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::RemoveDescriptorSetLayout(layoutName);
    }
}

void Renderer::CreateShaderUniformBuffer(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& dataRange /*= 0*/)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CreateUniformBufferInfo(bufferName, frameOverlap, bufferUsage, dataSize, dataRange);
    }
}

void Renderer::WriteShaderUniform(const std::string& name, const std::string& layoutName,const uint32_t& binding ,const bool& frameOverlap,const std::string& bufferName, const size_t& byteOffset /*=0*/)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CreateDescriptorSet(name, layoutName, binding,frameOverlap,bufferName, byteOffset);
    }
}

void Renderer::WriteShaderImage(const std::string& name, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CreateDescriptorSetImage(name, layoutName, binding, sampler, views); 
    }
}


void Renderer::RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::RemoveAllocatedBuffer(bufferName, frameOverlap);
    }
}

void Renderer::CreateSampler(const std::string& samplerName, const ImageFilter& filter, const SamplerAddressMode& samplerAddressMode)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CreateSampler(samplerName, (VkFilter)filter, (VkSamplerAddressMode) samplerAddressMode);
    }   
}

void Renderer::DestroySampler(const std::string& samplerName)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::DestroySampler(samplerName);
    }   
}

void Renderer::CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const ShaderDescriptions* descriptions /*= nullptr*/)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        //for now custom renderpasses are null since we don't yet support offscreen rendering
        if(descriptions == nullptr)
        {
            VkShaderDescriptions vkDescriptions;
            VulkanContext::CreateGraphicsPipeline(shaderPaths, shaderName, layoutNames, vkDescriptions, 0);
        }
        else
        {
            VkShaderDescriptions vkDescriptions;
            vkDescriptions.colorBlending = descriptions->colorBlending;
            vkDescriptions.cullMode = (VkCullModeFlagBits)descriptions->cullMode;
            vkDescriptions.depthCompareType = (VkCompareOp)descriptions->depthCompareType;
            vkDescriptions.depthTesting = descriptions->depthTesting;
            vkDescriptions.polygonMode = (VkPolygonMode)descriptions->polygonMode;
            vkDescriptions.vertexLocations.resize(descriptions->vertexLocations.size());
            for(int i = 0; i < descriptions->vertexLocations.size(); i++)
            {
                vkDescriptions.vertexLocations[i].format = (VkFormat)descriptions->vertexLocations[i].format;
                vkDescriptions.vertexLocations[i].offset = descriptions->vertexLocations[i].offset;
            }
            VulkanContext::CreateGraphicsPipeline(shaderPaths, shaderName, layoutNames,vkDescriptions, 0);
        }
    }
}

void Renderer::BindShader(const std::string& shaderName)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::BindGraphicsPipeline(shaderName);
    }   
}
void Renderer::BindUniforms(const std::string& name, const uint32_t& set, const uint32_t& dynamicOffset /*= 0*/ )
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::BindDescriptorSet(name, set,dynamicOffset);
    }
}

void Renderer::UpdateRenderer(std::array<float, 4> clearColor, std::function<void()>&& drawCalls)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::UpdateDraw(clearColor.data(), [=]{drawCalls();});
    } 
}

void Renderer::BindVertexBuffer(AllocatedBuffer& vertexBuffer)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::BindVertexBuffer(vertexBuffer);
    }
}

void Renderer::BindIndexBuffer(AllocatedBuffer& indexBuffer)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::BindIndexBuffer(indexBuffer);
    }
}


void Renderer::DrawIndexed(std::vector<std::uint32_t>& indices, const uint32_t& currentInstance)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::DrawIndexed(indices, currentInstance);
    } 
}

void Renderer::PrepareIndirectDraw(const uint32_t& MAX_COMMANDS)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::PrepareIndirectDraw(MAX_COMMANDS);
    }
}

void Renderer::UploadIndirectDraw(const uint32_t& objectCount, const std::vector<uint32_t>& indexSize, const uint32_t& currentInstance)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::UploadIndirectDraw(objectCount, indexSize, currentInstance);
    }
}

void Renderer::DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::DrawIndexedIndirect(drawCount, drawIndex);
    }
}

void Renderer::CleanUpRenderer(FunctionQueuer* p_additionalDeletion /*= nullptr*/)
{
    if(currentBackend == AvailableBackends::Vulkan)
    {
        VulkanContext::CleanUpVulkan(p_additionalDeletion);
    }
}

