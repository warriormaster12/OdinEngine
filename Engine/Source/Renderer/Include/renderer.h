#pragma once 

#include "vk_context.h"
#include "function_queuer.h"

#include <array>
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

typedef enum UniformType {
    UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER = 1,
    UNIFORM_TYPE_UNIFORM_BUFFER = 6,
    UNIFORM_TYPE_STORAGE_BUFFER = 7,
    UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
    UNIFORM_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
} UniformType;

typedef enum BufferUsageFlagBits {
    BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
    BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
    BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
    BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
    BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100,
} BufferUsageFlagBits;

typedef enum ShaderStageFlagBits {
    SHADER_STAGE_VERTEX_BIT = 0x00000001,
    SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
    SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
    SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
    SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
    SHADER_STAGE_COMPUTE_BIT = 0x00000020,
    SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
    SHADER_STAGE_ALL = 0x7FFFFFFF,
} ShaderStageFlagBits;

typedef enum ImageFilter {
    FILTER_NEAREST = 0,
    FILTER_LINEAR = 1,
    FILTER_CUBIC_IMG = 1000015000,
    FILTER_MAX_ENUM = 0x7FFFFFFF
} ImageFilter;
enum  class ColorFormat
{
    SRGB32 = VK_FORMAT_R32G32B32_SFLOAT,
    SRG32 = VK_FORMAT_R32G32_SFLOAT
};
#define SRGB32 (VkFormat)ColorFormat::SRGB32
#define SRG32 (VkFormat)ColorFormat::SRG32

typedef uint32_t Flags;
typedef Flags ShaderStageFlags;
typedef Flags BufferCreateFlags;

namespace Renderer
{
    void InitRenderer(AvailableBackends selectBackend);
    void UpdateRenderer(std::array<float, 4> clearColor, std::function<void()>&& drawCalls);
    void CleanUpRenderer(FunctionQueuer* p_additionalDeletion = nullptr);

    void CreateFramebuffer(ObjectType type);
    void CreateRenderPass(ObjectType type);

    //Equivalent to creating a descriptor set layout 
    void CreateShaderUniformLayoutBinding(const UniformType& uniformType, const ShaderStageFlags& shaderStage,const uint32_t& binding, const uint32_t& writeCount = 1);
    void CreateShaderUniformLayout(const std::string& layoutName);
    void RemoveShaderUniformLayout(const std::string& layoutName);

    void CreateSampler(const std::string& samplerName, const ImageFilter& filter);
    
    void DestroySampler(const std::string& samplerName);

    //create the shader
    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const ShaderDescriptions* descriptions = nullptr);
    //Equivalent to writing a descriptor set
    void CreateShaderUniformBuffer(const std::string& bufferName, const bool& frameOverlap,const BufferCreateFlags& bufferUsage, const size_t& dataSize, const size_t& byteOffset=0);
    void WriteShaderUniform(const std::string& name, const std::string& layoutName,const uint32_t& binding ,const bool& frameOverlap, const std::string& bufferName);
    void WriteShaderImage(const std::string& name, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views);
    void RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap);
    //Bind the shader before drawing
    void BindShader(const std::string& shaderName);

    

    

    //Equivalent to binding descriptor sets
    void BindUniforms(const std::string& name, const std::string& shaderName, const uint32_t& set,const bool& frameOverlap, const bool& isDynamic = false, const size_t& dataSize =0);

    void BindVertexBuffer(AllocatedBuffer& vertexBuffer);
    
    void BindIndexBuffer(AllocatedBuffer& indexBuffer);

    void DrawIndexed(std::vector<std::uint32_t>& indices);

    AvailableBackends GetActiveAPI();

    //template functions 

    template<typename T>
    void UploadUniformDataToShader(const std::string& bufferName, const T& data, const bool& frameOverlap)
    {
        if(GetActiveAPI() == AvailableBackends::Vulkan)
        {
            VulkanContext::UploadBufferData(bufferName, data, frameOverlap);
        }
    }
};
