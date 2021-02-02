#include "Include/vk_shaderhandler.h"
#include "../../Logger/Include/logger.h"
#include <iostream>
#include <fstream>



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
            std::cout << "Failed to load shader: " << filename << std::endl;
            throw std::runtime_error("failed to open file: " + filename);
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
            std::cout << "GLSL Preprocessing Failed for: " << filename << std::endl;
            std::cout << Shader.getInfoLog() << std::endl;
            std::cout << Shader.getInfoDebugLog() << std::endl;
        }
        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);
        if (!Shader.parse(&Resources, 100, false, messages))
        {
            std::cout << "GLSL Parsing Failed for: " << filename << std::endl;
            std::cout << Shader.getInfoLog() << std::endl;
            std::cout << Shader.getInfoDebugLog() << std::endl;
        }
        glslang::TProgram Program;
        Program.addShader(&Shader);

        if(!Program.link(messages))
        {
            std::cout << "GLSL Linking Failed for: " << filename << std::endl;
            std::cout << Shader.getInfoLog() << std::endl;
            std::cout << Shader.getInfoDebugLog() << std::endl;
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

std::string vkcomponent::GetFilePath(const std::string& str)
{
    size_t found = str.find_last_of("/\\");
    return str.substr(0,found);
    //size_t FileName = str.substr(found+1);
}

bool vkcomponent::LoadShaderModule(const char* p_filePath, VkShaderModule* p_outShaderModule, VkDevice& device)
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
		return false;
	}
	*p_outShaderModule = shaderModule;
	return true;
}
    
