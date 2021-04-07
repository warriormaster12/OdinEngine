#include "Include/renderer.h"
#include "logger.h"


AvailableBackends currentBackend;

namespace Renderer
{
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

    void WriteShaderUniform(const std::string& name, AllocatedBuffer& allocatedBuffer, const size_t& dataSize, size_t byteOffset /*= 0*/)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateDescriptorSet(name, allocatedBuffer, dataSize, byteOffset);
        }
    }

    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::RemoveAllocatedBuffer(allocatedBuffer);
        }
    }

    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::string& layoutName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::CreateGraphicsPipeline(shaderPaths, shaderName, layoutName);
        }
    }

    void BindShader(const std::string& shaderName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindGraphicsPipeline(shaderName);
        }   
    }
    void BindUniforms(const std::string& name, const std::string& shaderName)
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::BindDescriptorSet(name, shaderName);
        }
    }

    void UpdateRenderer(std::array<float, 4> clearColor, std::function<void()>&& drawCalls)
    {
       if(currentBackend == AvailableBackends::Vulkan)
        {
            float colors[4] = {clearColor[0], clearColor[1], clearColor[2], clearColor[3]};
            
            VulkanContext::UpdateDraw(colors, [=]{drawCalls();});
        } 
    }
    void Draw()
    {
        if(currentBackend == AvailableBackends::Vulkan)
        {
            VulkanContext::Draw();
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
