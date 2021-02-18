#pragma once 

#include "vk_types.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include "spirv_reflect.h"
#include "vk_descriptors.h"

namespace vkcomponent
{
    struct ShaderModule {
        std::vector<uint32_t> code;
        VkShaderModule module;
    };

    //loads a shader module from a spir-v file. Returns false if it errors
    bool LoadShaderModule(const char* p_filePath, ShaderModule* p_outShaderModule, VkDevice& device);
    uint32_t HashDescriptorLayoutInfo(VkDescriptorSetLayoutCreateInfo* info);
    
    std::string GetSuffix(const std::string& name);
    EShLanguage GetShaderStage(const std::string& stage);
    const std::string CompileGLSL(const std::string& filename);
    std::string GetFilePath(const std::string& str);

    TBuiltInResource HandleResources();

    struct ShaderEffect {

        struct ReflectionOverrides {
            const char* name;
            VkDescriptorType overridenType;
        };

        void AddStage(ShaderModule* shaderModule, VkShaderStageFlagBits stage);

        void ReflectLayout(VkDevice& device, ReflectionOverrides* overrides, int overrideCount);
        void FlushLayout();

        void FillStages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages);
        VkPipelineLayout builtLayout;

        struct ReflectedBinding {
            uint32_t set;
            uint32_t binding;
            VkDescriptorType type;
        };
        std::unordered_map<std::string, ReflectedBinding> bindings;
        std::array<VkDescriptorSetLayout, 4> setLayouts;
        std::array<uint32_t, 4> setHashes;
    private:
        struct ShaderStage {
            ShaderModule* shaderModule;
            VkShaderStageFlagBits stage;
        };

        std::vector<ShaderStage> stages;
    };
}