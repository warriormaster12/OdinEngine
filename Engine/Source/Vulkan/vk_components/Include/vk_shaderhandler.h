#pragma once 

#include "vk_types.h"
#include "vk_init.h"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include "spirv_reflect.h"
#include <StandAlone/DirStackFileIncluder.h>
#include "vk_descriptors.h"

namespace vkcomponent
{
    struct ShaderModule {
        std::vector<uint32_t> code;
        VkShaderModule module;
    };
    //loads a shader module from a spir-v file. Returns false if it errors
    bool LoadShaderModule(const char* p_filePath, VkShaderModule* p_outShaderModule, VkDevice& device);
    
    std::string GetSuffix(const std::string& name);
    EShLanguage GetShaderStage(const std::string& stage);
    const std::string CompileGLSL(const std::string& filename);
    std::string GetFilePath(const std::string& str);

    TBuiltInResource HandleResources();

    uint32_t HashDescriptorLayoutInfo(VkDescriptorSetLayoutCreateInfo* info);

    struct ShaderEffect {

        struct ReflectionOverrides {
            const char* name;
            VkDescriptorType overridenType;
        };

        void AddStage(ShaderModule* shaderModule, VkShaderStageFlagBits stage);

        void ReflectLayout(VkDevice device, ReflectionOverrides* overrides, int overrideCount);

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

    struct ShaderDescriptorBinder {
        
        struct BufferWriteDescriptor {
            int dstSet;
            int dstBinding;
            VkDescriptorType descriptorType;
            VkDescriptorBufferInfo bufferInfo;

            uint32_t dynamic_offset;
        };	

        void BindBuffer(const char* name, const VkDescriptorBufferInfo& bufferInfo);

        void BindDynamicBuffer(const char* name, uint32_t offset,const VkDescriptorBufferInfo& bufferInfo);

        void ApplyBinds( VkCommandBuffer cmd);

        //void build_sets(VkDevice device, VkDescriptorPool allocator);
        void BuildSets(VkDevice device, DescriptorAllocator& allocator);

        void SetShader(ShaderEffect* newShader);

        std::array<VkDescriptorSet, 4> cachedDescriptorSets;
    private:
        struct DynOffsets {
            std::array<uint32_t, 16> offsets;
            uint32_t count{ 0 };
        };
        std::array<DynOffsets, 4> setOffsets;

        ShaderEffect* shaders{ nullptr };
        std::vector<BufferWriteDescriptor> bufferWrites;
    };

    class ShaderCache {

    public:

        ShaderModule* GetShader(const std::string& path);

        void Init(VkDevice& device) { p_device = &device; };
    private:
        VkDevice* p_device;
        std::unordered_map<std::string, ShaderModule> module_cache;
    };
}