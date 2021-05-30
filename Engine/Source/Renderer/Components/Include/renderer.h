#pragma once 

#include "function_queuer.h"
#include "vk_context.h"

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
typedef enum SamplerAddressMode {
    SAMPLER_ADDRESS_MODE_REPEAT = 0,
    SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
    SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
    SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3,
    SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
    SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR = SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
    SAMPLER_ADDRESS_MODE_MAX_ENUM = 0x7FFFFFFF
} SamplerAddressMode;
typedef enum ColorFormat
{
    SRGB32 = 106,
    SRG32 = 103,
    SRGB8 = 43,
} ColorFormat;
typedef enum CompareOp {
    COMPARE_OP_NEVER = 0,
    COMPARE_OP_LESS = 1,
    COMPARE_OP_EQUAL = 2,
    COMPARE_OP_LESS_OR_EQUAL = 3,
    COMPARE_OP_GREATER = 4,
    COMPARE_OP_NOT_EQUAL = 5,
    COMPARE_OP_GREATER_OR_EQUAL = 6,
    COMPARE_OP_ALWAYS = 7
} CompareOp;
typedef enum CullModeFlagBits {
    CULL_MODE_NONE = 0,
    CULL_MODE_FRONT_BIT = 1,
    CULL_MODE_BACK_BIT = 2,
    CULL_MODE_FRONT_AND_BACK = 0x00000003
} CullModeFlagBits;

typedef enum PolygonMode {
    POLYGON_MODE_FILL = 0,
    POLYGON_MODE_LINE = 1,
    POLYGON_MODE_POINT = 2,
    POLYGON_MODE_FILL_RECTANGLE_NV = 1000153000,
    POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF
} PolygonMode;

typedef uint32_t Flags;
typedef Flags CullModeFlags;
typedef Flags ShaderStageFlags;
typedef Flags BufferCreateFlags;


struct VertexLocationInfo
{
	ColorFormat format;
	uint32_t offset;
};

struct ShaderDescriptions
{
    std::vector <VertexLocationInfo> vertexLocations;

    bool colorBlending = true;
    PolygonMode polygonMode = POLYGON_MODE_FILL;
    CullModeFlags cullMode = CULL_MODE_NONE;

    bool depthTesting = false;
    CompareOp depthCompareType = COMPARE_OP_EQUAL;
};

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

    void CreateSampler(const std::string& samplerName, const ImageFilter& filter, const SamplerAddressMode& samplerAddressMode);
    
    void DestroySampler(const std::string& samplerName);

    //create the shader
    void CreateShader(std::vector<std::string> shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const ShaderDescriptions* descriptions = nullptr);
    //Equivalent to writing a descriptor set
    void CreateShaderUniformBuffer(const std::string& bufferName, const bool& frameOverlap,const BufferCreateFlags& bufferUsage, const size_t& dataSize, const size_t& dataRange = 0);
    void WriteShaderUniform(const std::string& name, const std::string& layoutName,const uint32_t& binding ,const bool& frameOverlap, const std::string& bufferName, const size_t& byteOffset =0);
    void WriteShaderImage(const std::string& name, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views);
    void RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap);
    //Bind the shader before drawing
    void BindShader(const std::string& shaderName);

    

    

    //Equivalent to binding descriptor sets
    void BindUniforms(const std::string& name, const uint32_t& set, const uint32_t& dynamicOffset = 0, const bool& frameOverlap = false);

    void BindVertexBuffer(AllocatedBuffer& vertexBuffer);
    
    void BindIndexBuffer(AllocatedBuffer& indexBuffer);

    void DrawIndexed(std::vector<std::uint32_t>& indices, const uint32_t& currentInstance);

    void PrepareIndirectDraw(const uint32_t& MAX_COMMANDS);
    
    void UploadIndirectDraw(const uint32_t& objectCount, const std::vector<uint32_t>& indexSize, const uint32_t& currentInstance);
    
    void DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex);

    AvailableBackends GetActiveAPI();

    //template functions 

    template<typename T>
    void UploadSingleUniformDataToShader(const std::string& bufferName, const T& data, const bool& frameOverlap, const size_t& byteOffset = 0)
    {
        if(GetActiveAPI() == AvailableBackends::Vulkan)
        {
            VulkanContext::UploadSingleBufferData(bufferName, data,frameOverlap, byteOffset);
        }
    }
    template<typename T>
    void UploadVectorUniformDataToShader(const std::string& bufferName, const std::vector<T>& data, const bool& frameOverlap)
    {
        if(GetActiveAPI() == AvailableBackends::Vulkan)
        {
            VulkanContext::UploadVectorBufferData(bufferName, data,frameOverlap);
        }
    }
};
