#include "Include/vk_shaderhandler.h"
#include "logger.h"
#include "vk_init.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "function_queuer.h"


FunctionQueuer descriptorDeletionQueue;
bool glslangInitialized = false;
  
TBuiltInResource vkcomponent::HandleResources()
{
    TBuiltInResource DefaultTBuiltInResource {};
    DefaultTBuiltInResource.maxLights =  32;
    DefaultTBuiltInResource.maxClipPlanes =  6;
    DefaultTBuiltInResource.maxTextureUnits =  32;
    DefaultTBuiltInResource.maxTextureCoords =  32;
    DefaultTBuiltInResource.maxVertexAttribs =  64;
    DefaultTBuiltInResource.maxVertexUniformComponents =  4096;
    DefaultTBuiltInResource.maxVaryingFloats =  64;
    DefaultTBuiltInResource.maxVertexTextureImageUnits =  32;
    DefaultTBuiltInResource.maxCombinedTextureImageUnits =  80;
    DefaultTBuiltInResource.maxTextureImageUnits =  32;
    DefaultTBuiltInResource.maxFragmentUniformComponents =  4096;
    DefaultTBuiltInResource.maxDrawBuffers =  32;
    DefaultTBuiltInResource.maxVertexUniformVectors =  128;
    DefaultTBuiltInResource.maxVaryingVectors =  8;
    DefaultTBuiltInResource.maxFragmentUniformVectors =  16;
    DefaultTBuiltInResource.maxVertexOutputVectors =  16;
    DefaultTBuiltInResource.maxFragmentInputVectors =  15;
    DefaultTBuiltInResource.minProgramTexelOffset =  -8;
    DefaultTBuiltInResource.maxProgramTexelOffset =  7;
    DefaultTBuiltInResource.maxClipDistances =  8;
    DefaultTBuiltInResource.maxComputeWorkGroupCountX =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupCountY =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupCountZ =  65535;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeX =  1024;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeY =  1024;
    DefaultTBuiltInResource.maxComputeWorkGroupSizeZ =  64;
    DefaultTBuiltInResource.maxComputeUniformComponents =  1024;
    DefaultTBuiltInResource.maxComputeTextureImageUnits =  16;
    DefaultTBuiltInResource.maxComputeImageUniforms =  8;
    DefaultTBuiltInResource.maxComputeAtomicCounters =  8;
    DefaultTBuiltInResource.maxComputeAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxVaryingComponents =  60;
    DefaultTBuiltInResource.maxVertexOutputComponents =  64;
    DefaultTBuiltInResource.maxGeometryInputComponents =  64;
    DefaultTBuiltInResource.maxGeometryOutputComponents =  128;
    DefaultTBuiltInResource.maxFragmentInputComponents =  128;
    DefaultTBuiltInResource.maxImageUnits =  8;
    DefaultTBuiltInResource.maxCombinedImageUnitsAndFragmentOutputs =  8;
    DefaultTBuiltInResource.maxCombinedShaderOutputResources =  8;
    DefaultTBuiltInResource.maxImageSamples =  0;
    DefaultTBuiltInResource.maxVertexImageUniforms =  0;
    DefaultTBuiltInResource.maxTessControlImageUniforms =  0;
    DefaultTBuiltInResource.maxTessEvaluationImageUniforms =  0;
    DefaultTBuiltInResource.maxGeometryImageUniforms =  0;
    DefaultTBuiltInResource.maxFragmentImageUniforms =  8;
    DefaultTBuiltInResource.maxCombinedImageUniforms =  8;
    DefaultTBuiltInResource.maxGeometryTextureImageUnits =  16;
    DefaultTBuiltInResource.maxGeometryOutputVertices =  256;
    DefaultTBuiltInResource.maxGeometryTotalOutputComponents =  1024;
    DefaultTBuiltInResource.maxGeometryUniformComponents =  1024;
    DefaultTBuiltInResource.maxGeometryVaryingComponents =  64;
    DefaultTBuiltInResource.maxTessControlInputComponents =  128;
    DefaultTBuiltInResource.maxTessControlOutputComponents =  128;
    DefaultTBuiltInResource.maxTessControlTextureImageUnits =  16;
    DefaultTBuiltInResource.maxTessControlUniformComponents =  1024;
    DefaultTBuiltInResource.maxTessControlTotalOutputComponents =  4096;
    DefaultTBuiltInResource.maxTessEvaluationInputComponents =  128;
    DefaultTBuiltInResource.maxTessEvaluationOutputComponents =  128;
    DefaultTBuiltInResource.maxTessEvaluationTextureImageUnits =  16;
    DefaultTBuiltInResource.maxTessEvaluationUniformComponents =  1024;
    DefaultTBuiltInResource.maxTessPatchComponents =  120;
    DefaultTBuiltInResource.maxPatchVertices =  32;
    DefaultTBuiltInResource.maxTessGenLevel =  64;
    DefaultTBuiltInResource.maxViewports =  16;
    DefaultTBuiltInResource.maxVertexAtomicCounters =  0;
    DefaultTBuiltInResource.maxTessControlAtomicCounters =  0;
    DefaultTBuiltInResource.maxTessEvaluationAtomicCounters =  0;
    DefaultTBuiltInResource.maxGeometryAtomicCounters =  0;
    DefaultTBuiltInResource.maxFragmentAtomicCounters =  8;
    DefaultTBuiltInResource.maxCombinedAtomicCounters =  8;
    DefaultTBuiltInResource.maxAtomicCounterBindings =  1;
    DefaultTBuiltInResource.maxVertexAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxTessControlAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxTessEvaluationAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxGeometryAtomicCounterBuffers =  0;
    DefaultTBuiltInResource.maxFragmentAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxCombinedAtomicCounterBuffers =  1;
    DefaultTBuiltInResource.maxAtomicCounterBufferSize =  16384;
    DefaultTBuiltInResource.maxTransformFeedbackBuffers =  4;
    DefaultTBuiltInResource.maxTransformFeedbackInterleavedComponents =  64;
    DefaultTBuiltInResource.maxCullDistances =  8;
    DefaultTBuiltInResource.maxCombinedClipAndCullDistances =  8;
    DefaultTBuiltInResource.maxSamples =  4;
    DefaultTBuiltInResource.maxMeshOutputVerticesNV =  256;
    DefaultTBuiltInResource.maxMeshOutputPrimitivesNV =  512;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeX_NV =  32;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeY_NV=  1;
    DefaultTBuiltInResource.maxMeshWorkGroupSizeZ_NV=  1;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeX_NV=  32;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeY_NV=  1;
    DefaultTBuiltInResource.maxTaskWorkGroupSizeZ_NV=  1;
    DefaultTBuiltInResource.maxMeshViewCountNV=  4;
    DefaultTBuiltInResource.limits.nonInductiveForLoops =  1;
    DefaultTBuiltInResource.limits.whileLoops =  1;
    DefaultTBuiltInResource.limits.doWhileLoops =  1;
    DefaultTBuiltInResource.limits.generalUniformIndexing =  1;
    DefaultTBuiltInResource.limits.generalAttributeMatrixVectorIndexing =  1;
    DefaultTBuiltInResource.limits.generalVaryingIndexing =  1;
    DefaultTBuiltInResource.limits.generalSamplerIndexing =  1;
    DefaultTBuiltInResource.limits.generalVariableIndexing =  1;
    DefaultTBuiltInResource.limits.generalConstantMatrixVectorIndexing =  1;

    return DefaultTBuiltInResource;
}


std::string vkcomponent::GetSuffix(const std::string& name)
{
    const size_t pos = name.rfind('.');
    return (pos == std::string::npos) ? "" : name.substr(name.rfind('.') + 1);
}
EShLanguage vkcomponent::GetShaderStage(const std::string& stage)
{
    if (stage == "vert") {
        return EShLangVertex;
    } else if (stage == "tesc") {
        return EShLangTessControl;
    } else if (stage == "tese") {
        return EShLangTessEvaluation;
    } else if (stage == "geom") {
        return EShLangGeometry;
    } else if (stage == "frag") {
        return EShLangFragment;
    } else if (stage == "comp") {
        return EShLangCompute;
    } else {
        assert(0 && "Unknown shader stage");
        return EShLangCount;
    }
}

VkShaderStageFlagBits vkcomponent::GetShaderStageVk(const std::string& stage)
{
    if (stage == "vert") {
        return VK_SHADER_STAGE_VERTEX_BIT;
    } else if (stage == "tesc") {
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (stage == "tese") {
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (stage == "geom") {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (stage == "frag") {
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (stage == "comp") {
        return VK_SHADER_STAGE_COMPUTE_BIT;
    } else {
        assert(0 && "Unknown shader stage");
        return VK_SHADER_STAGE_ALL;
    }
}

const std::string vkcomponent::CompileGLSL(const std::string& filename)
{
    std::string SpirV_filename = filename + ".spv";
    //Check that spv file already exists
    std::ifstream file_spirv(SpirV_filename);
    if (file_spirv.fail())
    {
        if (!glslangInitialized)
        {
            glslang::InitializeProcess();
            glslangInitialized = true;
        }
        //Load GLSL into a string
        std::ifstream file_glsl(filename);

        if (!file_glsl.is_open()) 
        {
            ENGINE_CORE_ERROR("Failed to load shader: {0}", filename);
            abort();
        }

        std::string InputGLSL((std::istreambuf_iterator<char>(file_glsl)),
                                std::istreambuf_iterator<char>());

        const char* InputCString = InputGLSL.c_str();

        EShLanguage ShaderType = GetShaderStage(GetSuffix(filename));
        glslang::TShader Shader(ShaderType);
        
        Shader.setStrings(&InputCString, 1);
        

        int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
        glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
        glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

        Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

        TBuiltInResource Resources;
        Resources = HandleResources();
        EShMessages messages = (EShMessages) (EShMsgSpvRules | EShMsgVulkanRules);

        const int DefaultVersion = 100;

        DirStackFileIncluder Includer;

        //Get Path of File
        std::string Path = GetFilePath(filename);
        Includer.pushExternalLocalDirectory(Path);

        std::string PreprocessedGLSL;

        if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, Includer)) 
        {
            ENGINE_CORE_ERROR("GLSL Preprocessing Failed for: {0}", filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);
        if (!Shader.parse(&Resources, 100, false, messages))
        {
            ENGINE_CORE_ERROR("GLSL Parsing Failed for: {0}",filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        glslang::TProgram Program;
        Program.addShader(&Shader);

        if(!Program.link(messages))
        {
            ENGINE_CORE_ERROR("GLSL Linking Failed for: {0} ", filename);
            ENGINE_CORE_ERROR(Shader.getInfoLog());
            ENGINE_CORE_ERROR(Shader.getInfoDebugLog());
        }
        std::vector<unsigned int> SpirV;
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);
        glslang::OutputSpvBin(SpirV, SpirV_filename.c_str());  
        ENGINE_CORE_INFO("file " + filename + " compiled to " + SpirV_filename);
        return SpirV_filename;
    }
    else 
    {
        ENGINE_CORE_INFO("file " + SpirV_filename + " already exists");
        return SpirV_filename;
    }
}
// FNV-1a 32bit hashing algorithm.
constexpr uint32_t fnv1a_32(char const* s, std::size_t count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}
uint32_t vkcomponent::HashDescriptorLayoutInfo(VkDescriptorSetLayoutCreateInfo* info)
{
	//we are going to put all the data into a string and then hash the string
	std::stringstream ss;

	ss << info->flags;
	ss << info->bindingCount;

	for (auto i = 0u; i < info->bindingCount; i++) {
		const VkDescriptorSetLayoutBinding &binding = info->pBindings[i];

		ss << binding.binding;
		ss << binding.descriptorCount;
		ss << binding.descriptorType;
		ss << binding.stageFlags;
	}

	auto str = ss.str();

	return fnv1a_32(str.c_str(),str.length());
}

std::string vkcomponent::GetFilePath(const std::string& str)
{
    size_t found = str.find_last_of("/\\");
    return str.substr(0,found);
    //size_t FileName = str.substr(found+1);
}

bool vkcomponent::LoadShaderModule(const char* p_filePath, ShaderModule* p_outShaderModule, VkDevice& device)
{
	//open the file. With cursor at the end
	std::ifstream file(p_filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at beggining
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        ENGINE_CORE_ERROR("Error when building the {}", p_filePath);
		return false;
	}
    std::string spirvName = p_filePath;
    std::string glslShaderName = spirvName.substr(0, spirvName.find_last_of("."));
    p_outShaderModule->code = std::move(buffer);
	p_outShaderModule->module = shaderModule;
    p_outShaderModule->stage = GetShaderStageVk(GetSuffix(glslShaderName));
	return true;
}
    


void vkcomponent::ShaderEffect::AddStage(ShaderModule* shaderModule, VkShaderStageFlagBits stage)
{
	ShaderStage newStage = { shaderModule,stage };
	stages.push_back(newStage);
}


void vkcomponent::ShaderEffect::FillStages(std::vector<VkPipelineShaderStageCreateInfo>& pipelineStages)
{
	for (auto& s : stages)
	{
		pipelineStages.push_back(vkinit::PipelineShaderStageCreateInfo(s.stage, s.shaderModule->module));
	}
}

void vkcomponent::ShaderEffect::FlushShaders(VkDevice& device)
{
    for(auto& currentStage : stages)
    {
        vkDestroyShaderModule(device, currentStage.shaderModule->module, nullptr);
    }
}


struct DescriptorSetLayoutData {
	uint32_t set_number;
	VkDescriptorSetLayoutCreateInfo create_info;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

void vkcomponent::ShaderEffect::ReflectLayout(VkDevice& device, ReflectionOverrides* overrides, int overrideCount)
{
    std::vector<DescriptorSetLayoutData> set_layouts;

	std::vector<VkPushConstantRange> constant_ranges;

	for (auto& s : stages) {	

		SpvReflectShaderModule spvmodule;
		SpvReflectResult result = spvReflectCreateShaderModule(s.shaderModule->code.size() * sizeof(uint32_t), s.shaderModule->code.data(), &spvmodule);
	
		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&spvmodule, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);	

		for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
			
			const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);

			DescriptorSetLayoutData layout = {};

			layout.bindings.resize(refl_set.binding_count);
			for (uint32_t i_binding = 0; i_binding < refl_set.binding_count; ++i_binding) {
				const SpvReflectDescriptorBinding& refl_binding = *(refl_set.bindings[i_binding]);
				VkDescriptorSetLayoutBinding& layout_binding = layout.bindings[i_binding];
				layout_binding.binding = refl_binding.binding;
				layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);

				for (int ov = 0; ov < overrideCount; ov++)
				{
					if (strcmp(refl_binding.name, overrides[ov].name) == 0) {
						layout_binding.descriptorType = overrides[ov].overridenType;
					}
				}

				layout_binding.descriptorCount = 1;
				for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
					layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
				}
				layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(spvmodule.shader_stage);

				ReflectedBinding reflected;
				reflected.binding = layout_binding.binding;
				reflected.set = refl_set.set;
				reflected.type = layout_binding.descriptorType;

				bindings[refl_binding.name] = reflected;
			}
			layout.set_number = refl_set.set;
			layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout.create_info.bindingCount = refl_set.binding_count;
			layout.create_info.pBindings = layout.bindings.data();
            layout.create_info.flags = 0;

			set_layouts.push_back(layout);
		}

		//pushconstants	

		result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectBlockVariable*> pconstants(count);
		result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, pconstants.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		if (count > 0) {
			VkPushConstantRange pcs{};
			pcs.offset = pconstants[0]->offset;
			pcs.size = pconstants[0]->size;
			pcs.stageFlags = s.stage;

			constant_ranges.push_back(pcs);
		}
	}


	

	std::array<DescriptorSetLayoutData,4> merged_layouts;
	
	for (int i = 0; i < 4; i++) {

		DescriptorSetLayoutData &ly = merged_layouts[i];

		ly.set_number = i;

		ly.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		std::unordered_map<int,VkDescriptorSetLayoutBinding> binds;
		for (auto& s : set_layouts) {
			if (s.set_number == i) {
				for (auto& b : s.bindings)
				{
					auto it = binds.find(b.binding);
					if (it == binds.end())
					{
						binds[b.binding] = b;
						//ly.bindings.push_back(b);
					}
					else {
						//merge flags
						binds[b.binding].stageFlags |= b.stageFlags;
					}
					
				}
			}
		}
		for (auto [k, v] : binds)
		{
			ly.bindings.push_back(v);
		}
		//sort the bindings, for hash purposes
		std::sort(ly.bindings.begin(), ly.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {			
			return a.binding < b.binding;
		});


		ly.create_info.bindingCount = (uint32_t)ly.bindings.size();
		ly.create_info.pBindings = ly.bindings.data();
		ly.create_info.flags = 0;
		ly.create_info.pNext = 0;
		

		if (ly.create_info.bindingCount > 0) {
			setHashes[i] = HashDescriptorLayoutInfo(&ly.create_info);
			vkCreateDescriptorSetLayout(device, &ly.create_info, nullptr, &setLayouts[i]);
		}
		else {
			setHashes[i] = 0;
			setLayouts[i] = VK_NULL_HANDLE;
		}
        descriptorDeletionQueue.PushFunction([=]()
        {
            vkDestroyDescriptorSetLayout(device, setLayouts[i], nullptr );
        });
	}

	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();

	mesh_pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
	mesh_pipeline_layout_info.pushConstantRangeCount = (uint32_t)constant_ranges.size();

	std::array<VkDescriptorSetLayout,4> compactedLayouts;
	int s = 0;
	for (int i = 0; i < 4; i++) {
		if (setLayouts[i] != VK_NULL_HANDLE) {
			compactedLayouts[s] = setLayouts[i];
			s++;
		}
	}

	mesh_pipeline_layout_info.setLayoutCount = s;
	mesh_pipeline_layout_info.pSetLayouts = compactedLayouts.data();

	
	vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr, &builtLayout);

}

void vkcomponent::ShaderEffect::FlushLayout()
{
    descriptorDeletionQueue.Flush();
}