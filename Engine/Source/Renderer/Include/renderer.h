#pragma once 

#include "vk_context.h"
#include <array>
#include <functional>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define BACKEND_VULKAN AvailableBackends::Vulkan

#define FRAMEBUFFER_MAIN ObjectType::Main
#define FRAMEBUFFER_OFFSCREEN ObjectType::Offscreen

#define RENDERPASS_MAIN ObjectType::Main
#define RENDERPASS_OFFSCREEN ObjectType::Offscreen

enum class AvailableBackends
{
    Vulkan = 0,
};

enum class ObjectType
{
    Main = 0,
    Offscreen = 0
};

namespace Renderer
{
    void InitRenderer(AvailableBackends selectBackend);
    void UpdateRenderer(std::array<float, 4> clearColor, std::function<void()>&& drawCalls);
    void CleanUpRenderer();

    void CreateFramebuffer(ObjectType type);
    void CreateRenderPass(ObjectType type);

    //Equivalent to creating a descriptor set layout 
    void CreateShaderUniformLayoutBinding(const VkDescriptorType& uniformName, const VkShaderStageFlags& shaderStage,const uint32_t& binding, const uint32_t& writeCount = 1);
    void CreateShaderUniformLayout(const std::string& layoutName);
    void RemoveShaderUniformLayout(const std::string& layoutName);
    //create the shader
    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::string& layoutName);
    //Equivalent to writing a descriptor set
    void WriteShaderUniform(const std::string& name, AllocatedBuffer& allocatedBuffer, const size_t& dataSize, size_t byteOffset = 0);
    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer);
    //Bind the shader before drawing
    void BindShader(const std::string& shaderName);
    //Equivalent to binding descriptor sets
    void BindUniforms(const std::string& name, const std::string& shaderName);

    void Draw();
};