#pragma once 

#include "../../Include/vk_types.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>

namespace vkcomponent
{
    bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule, VkDevice& device);
    std::string GetSuffix(const std::string& name);
    EShLanguage GetShaderStage(const std::string& stage);

    static std::vector<char> readFile(const std::string& filename);
    

    const std::string CompileGLSL(const std::string& filename);
    std::string GetFilePath(const std::string& str);

    TBuiltInResource handleResources();
}