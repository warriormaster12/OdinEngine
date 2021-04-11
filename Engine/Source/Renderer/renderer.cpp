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
    void CreateShaderUniformLayoutBinding(const VkDescriptorType& uniformName, const VkShaderStageFlags& shaderStage,const uint32_t& binding, const uint32_t& writeCount /*= 1*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            DescripotrSetLayoutBindingInfo info = {};
            info.descriptorType = uniformName;
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

    void WriteShaderUniform(const std::string& name, const std::string& layoutName,const VkBufferCreateFlags& bufferUsage,AllocatedBuffer& allocatedBuffer, const size_t& dataSize, size_t byteOffset /*= 0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSet(name, layoutName, bufferUsage,allocatedBuffer, dataSize, byteOffset);
        }
    }

    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::RemoveAllocatedBuffer(allocatedBuffer);
        }
    }

    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateGraphicsPipeline(shaderPaths, shaderName, layoutNames);
        }
    }

    void BindShader(const std::string& shaderName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindGraphicsPipeline(shaderName);
        }   
    }
    void BindUniforms(const std::string& name, const std::string& shaderName, const uint32_t& set /*= 0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindDescriptorSet(name, shaderName, set);
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
