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

    void WriteShaderUniform(const std::string& name, const std::string& layoutName,const uint32_t& binding ,const BufferCreateFlags& bufferUsage,AllocatedBuffer& allocatedBuffer, const size_t& dataSize, const size_t& byteOffset /*=0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSet(name, layoutName, binding,bufferUsage,allocatedBuffer, dataSize, byteOffset);
        }
    }


    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::RemoveAllocatedBuffer(allocatedBuffer);
        }
    }

    void CreateSampler(const std::string& samplerName, const ImageFilter& filter)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateSampler(samplerName, (VkFilter)filter);
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
    void BindUniforms(const std::string& name, const std::string& shaderName, const uint32_t& set /*= 0*/, const bool& isDynamic /*= false*/, const size_t& dataSize /*=0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindDescriptorSet(name, shaderName, set, isDynamic, dataSize);
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

    void CleanUpRenderer()
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CleanUpVulkan();
        }
    }
}
