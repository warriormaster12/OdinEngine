#include "Include/material_manager.h"
#include "logger.h"
#include "vk_device.h"
#include "vk_utils.h"
#include "unordered_finder.h"
#include "function_queuer.h"

#include <unordered_map>
#include <vector>

std::unordered_map<std::string, Material> materials;

std::vector<std::string> materialNameList;
std::vector<uint32_t> deletedOffsets;




struct GPUMaterialData
{
    glm::vec4 albedo; //vec3
    glm::vec4 emission; //vec3

    glm::vec4 emissionPower; //float
    glm::vec4 metallic; //float
    glm::vec4 roughness; //float
    glm::vec4 ao; //float

    glm::vec4 metallicChannelIndex; //int
    glm::vec4 roughnessChannelIndex; //int

    glm::vec4 repeateCount;
}materialData;

const int maxMaterial = 30;
static FunctionQueuer textureQueuer;
void MaterialManager::Init()
{
    Renderer::CreateShaderUniformBuffer("material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, PadUniformBufferSize(sizeof(GPUMaterialData))* maxMaterial, sizeof(GPUMaterialData));
    Renderer::WriteShaderUniform("object data", "per object layout",1,false,"material buffer");

    CreateMaterial("default material");
}

void MaterialManager::CreateMaterial(const std::string& materialName)
{
    if(FindUnorderedMap(materialName, materials) == nullptr)
    {
        materials[materialName];
        if(deletedOffsets.size() !=0)
        {
            FindUnorderedMap(materialName, materials)->offset = deletedOffsets[deletedOffsets.size()-1];
            deletedOffsets.pop_back();
        }
        else {
            int currentIndex = materialNameList.size() % maxMaterial;
            FindUnorderedMap(materialName, materials)->offset = currentIndex * PadUniformBufferSize(sizeof(GPUMaterialData));
        }
        
        FindUnorderedMap(materialName, materials)->UpdateMaterial();
        materialNameList.push_back(materialName);
        std::vector<VkImageView> imageViews;
        imageViews.resize(6);
        Renderer::WriteShaderImage(materialName, "texture data layout", 0, "", imageViews);
        ENGINE_CORE_INFO("material by name {0} created", materialName);
    }
    else {
        ENGINE_CORE_WARN("{0} material already exists", materialName);
    }
}

void Material::ExecuteTextures()
{
    textureQueuer.Flush();
    textureUpdate = false;
}

void MaterialManager::AddTextures(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    FindUnorderedMap(materialName, materials)->UpdateTextures(true);
    FindUnorderedMap(materialName, materials)->textureObjects.resize(FindUnorderedMap(materialName, materials)->GetTextures().size());
    textureQueuer.PushFunction([=]{
        std::vector<VkImageView> imageViews;
        for(int i = 0; i < FindUnorderedMap(materialName, materials)->GetTextures().size(); i++)
        {
            FindUnorderedMap(materialName, materials)->textureCheck.textures[i] = 0;
            FindUnorderedMap(materialName, materials)->textureObjects[i].CreateTexture(FindUnorderedMap(materialName, materials)->GetTextures()[i], FindUnorderedMap(materialName, materials)->GetFormats()[i]);
            imageViews.push_back(FindUnorderedMap(materialName, materials)->textureObjects[i].image.defaultView);
            if(FindUnorderedMap(materialName, materials)->GetTextures()[i] != "")
            {
                FindUnorderedMap(materialName, materials)->textureCheck.textures[i] = 1;
            }
        }
        Renderer::WriteShaderImage(materialName, "texture data layout", 0, samplerName, imageViews);
    });
};

Material& MaterialManager::GetMaterial(const std::string& materialName)
{
    return *FindUnorderedMap(materialName, materials);
}

std::vector<std::string>& MaterialManager::GetMaterials()
{
    return materialNameList;
}

void MaterialManager::BindMaterial(const std::string& materialName)
{

    uint32_t offset = FindUnorderedMap(materialName, materials)->offset;
    if(FindUnorderedMap(materialName, materials)->isUpdated())
    {
        materialData.albedo = FindUnorderedMap(materialName, materials)->GetColor();
        materialData.roughness = glm::vec4(FindUnorderedMap(materialName, materials)->GetRoughness());
        materialData.metallic = glm::vec4(FindUnorderedMap(materialName, materials)->GetMetallic());
        materialData.ao = glm::vec4(FindUnorderedMap(materialName, materials)->GetAo());
        materialData.emission = glm::vec4(FindUnorderedMap(materialName, materials)->GetEmission());
        materialData.emissionPower = glm::vec4(FindUnorderedMap(materialName, materials)->GetEmissionPower());
        materialData.repeateCount = glm::vec4(FindUnorderedMap(materialName, materials)->GetRepeateCount());
        materialData.metallicChannelIndex = glm::vec4(FindUnorderedMap(materialName, materials)->GetMetallicColorChannel());
        materialData.roughnessChannelIndex = glm::vec4(FindUnorderedMap(materialName, materials)->GetRoughnessColorChannel());
        FindUnorderedMap(materialName, materials)->ResetUpdate();
        Renderer::UploadSingleUniformDataToShader("material buffer",materialData, false, offset);
    }
    Renderer::BindUniforms(materialName,2);
    Renderer::BindPushConstants(SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(TextureCheck), &FindUnorderedMap(materialName, materials)->textureCheck);
}

void MaterialManager::DeleteMaterial(const std::string& materialName)
{
    if(FindUnorderedMap(materialName, materials) != nullptr)
    {
        for(auto& currentTexture : FindUnorderedMap(materialName, materials)->textureObjects)
        {
            currentTexture.DestroyTexture();
        }
        for(int i = 0; i < materialNameList.size(); i++)
        {
            if(materialNameList[i] == materialName)
            {
                materialNameList.erase(materialNameList.begin() + i);
                materialNameList.shrink_to_fit();
            }
        }
        deletedOffsets.push_back(FindUnorderedMap(materialName, materials)->offset); 
        materials.erase(materialName);
    }
}

void MaterialManager::DeleteAllMaterials()
{
    for(auto& currentMaterialName : materialNameList)
    {
        if(FindUnorderedMap(currentMaterialName, materials) != nullptr)
        {
            for(auto& currentTexture : FindUnorderedMap(currentMaterialName, materials)->textureObjects)
            {
                currentTexture.DestroyTexture();
            }
            materials.erase(currentMaterialName);
        }
    }
    Renderer::RemoveAllocatedBuffer("material buffer", false);
}
