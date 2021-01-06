#pragma once 

#include "../../Include/vk_types.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>

namespace vkcomponent
{
    //loads a shader module from a spir-v file. Returns false if it errors
    bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule, VkDevice& device);
    
    std::string GetSuffix(const std::string& name);
    EShLanguage GetShaderStage(const std::string& stage);
    const std::string CompileGLSL(const std::string& filename);
    std::string GetFilePath(const std::string& str);

    TBuiltInResource handleResources();
}