#include "Include/renderer.h"
#include "logger.h"


AvailableBackends currentBackend;

namespace Renderer
{
    AvailableBackends GetActiveAPI()
    {
        return currentBackend;
    }
    void InitRenderer(AvailableBackends selectBackend)
    {
        currentBackend = selectBackend;
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::InitVulkan();
        }
    }

    void CreateFramebuffer(ObjectType type)
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

    void CreateRenderPass(ObjectType type)
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
    void CreateShaderUniformLayoutBinding(const UniformType& uniformType, const ShaderStageFlags& shaderStage,const uint32_t& binding, const uint32_t& writeCount /*= 1*/)
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

    void CreateShaderUniformLayout(const std::string& layoutName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSetLayout(layoutName);
        }
    }

    void RemoveShaderUniformLayout(const std::string& layoutName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::RemoveDescriptorSetLayout(layoutName);
        }
    }

    void CreateShaderUniformBuffer(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& byteOffset /*=0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateUniformBufferInfo(bufferName, frameOverlap, bufferUsage, dataSize, byteOffset);
        }
    }

    void WriteShaderUniform(const std::string& name, const std::string& layoutName,const uint32_t& binding ,const bool& frameOverlap,const std::string& bufferName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSet(name, layoutName, binding,frameOverlap,bufferName);
        }
    }

    void WriteShaderImage(const std::string& name, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSetImage(name, layoutName, binding, sampler, views); 
        }
    }


    void RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::RemoveAllocatedBuffer(bufferName, frameOverlap);
        }
    }

    void CreateSampler(const std::string& samplerName, const ImageFilter& filter)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateSampler(samplerName, (VkFilter)filter);
        }   
    }

    void DestroySampler(const std::string& samplerName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::DestroySampler(samplerName);
        }   
    }

    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const ShaderDescriptions* descriptions /*= nullptr*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            //for now custom renderpasses are null since we don't yet support offscreen rendering
            VulkanContext::CreateGraphicsPipeline(shaderPaths, shaderName, layoutNames, 0, descriptions);
        }
    }

    void BindShader(const std::string& shaderName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindGraphicsPipeline(shaderName);
        }   
    }
    void BindUniforms(const std::string& name, const std::string& shaderName, const uint32_t& set,  const bool& frameOverlap, const bool& isDynamic /*= false*/, const size_t& dataSize /*=0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindDescriptorSet(name, shaderName, set, frameOverlap,isDynamic, dataSize);
        }
    }

    void UpdateRenderer(std::array<float, 4> clearColor, std::function<void()>&& drawCalls)
    {
       if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::UpdateDraw(clearColor.data(), [=]{drawCalls();});
        } 
    }

    void BindVertexBuffer(AllocatedBuffer& vertexBuffer)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindVertexBuffer(vertexBuffer);
        }
    }
    
    void BindIndexBuffer(AllocatedBuffer& indexBuffer)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindIndexBuffer(indexBuffer);
        }
    }


    void DrawIndexed(std::vector<std::uint32_t>& indices)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::DrawIndexed(indices);
        } 
    }

    void CleanUpRenderer(FunctionQueuer* p_additionalDeletion /*= nullptr*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CleanUpVulkan(p_additionalDeletion);
        }
    }
}
