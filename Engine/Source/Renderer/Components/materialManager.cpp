#include "Include/materialManager.h"
#include "Include/renderer.h"
#include "logger.h"
#include "vk_utils.h"
#include "unordered_finder.h"

#include <bits/stdint-uintn.h>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, Material> materials;

std::vector<std::string> materialNameList;
std::vector<uint32_t> deletedOffsets;


struct GPUMaterialData
{
    glm::vec4 color;
    glm::vec4 repeateCount;
}materialData;

const int maxMaterial = 30;
void MaterialManager::Init()
{
    Renderer::CreateShaderUniformBuffer("material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, PadUniformBufferSize(sizeof(GPUMaterialData))* maxMaterial, sizeof(GPUMaterialData));
    Renderer::WriteShaderUniform("object data", "per object layout",1,false,"material buffer");
    
}

void MaterialManager::CreateMaterial(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    //TODO: figure out a way how to add offset without overwriting existing material
    if(FindUnorderdMap(materialName, materials) == nullptr)
    {
        materials[materialName];
        if(deletedOffsets.size() !=0)
        {
            FindUnorderdMap(materialName, materials)->offset = deletedOffsets[deletedOffsets.size()-1];
            deletedOffsets.pop_back();
        }
        else {
            int currentIndex = materialNameList.size() % maxMaterial;
            FindUnorderdMap(materialName, materials)->offset = currentIndex * PadUniformBufferSize(sizeof(GPUMaterialData));
        }
        
        FindUnorderdMap(materialName, materials)->SetColor(glm::vec4(1.0));
        FindUnorderdMap(materialName, materials)->SetRepeateCount(1);
        materialNameList.push_back(materialName);
        ENGINE_CORE_INFO("material by name {0} created", materialName);
    }
    else {
        ENGINE_CORE_WARN("{0} material already exists", materialName);
    }
}

void  MaterialManager::AddTextures(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    FindUnorderdMap(materialName, materials)->textureObjects.resize(FindUnorderdMap(materialName, materials)->GetTextures().size());
    std::vector<VkImageView> views;
    for(int i = 0; i < FindUnorderdMap(materialName, materials)->GetTextures().size(); i++)
    {
        FindUnorderdMap(materialName, materials)->textureObjects[i].CreateTexture(FindUnorderdMap(materialName, materials)->GetTextures()[i]);
        views.push_back(FindUnorderdMap(materialName, materials)->textureObjects[i].imageView);
    }
    Renderer::WriteShaderImage(materialName, "texture data layout", 0, samplerName, views);
};

Material& MaterialManager::GetMaterial(const std::string& materialName)
{
    return *FindUnorderdMap(materialName, materials);
}

void MaterialManager::BindMaterial(const std::string& materialName)
{

    uint32_t offset = FindUnorderdMap(materialName, materials)->offset;
    if(FindUnorderdMap(materialName, materials)->isUpdated())
    {
        materialData.color = FindUnorderdMap(materialName, materials)->GetColor();
        materialData.repeateCount = glm::vec4(FindUnorderdMap(materialName, materials)->GetRepeateCount());
        FindUnorderdMap(materialName, materials)->ResetUpdate();
        Renderer::UploadSingleUniformDataToShader("material buffer",materialData, false, offset);
    }

    if(FindUnorderdMap(materialName, materials)->textureObjects.size() != 0)
    {
        Renderer::BindUniforms(materialName,2,offset);
    } 
}

void MaterialManager::DeleteMaterial(const std::string& materialName)
{
    if(FindUnorderdMap(materialName, materials) != nullptr)
    {
        for(auto& currentTexture : FindUnorderdMap(materialName, materials)->textureObjects)
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
        deletedOffsets.push_back(FindUnorderdMap(materialName, materials)->offset); 
        materials.erase(materialName);
    }
}

void MaterialManager::DeleteAllMaterials()
{
    for(auto& currentMaterialName : materialNameList)
    {
        if(FindUnorderdMap(currentMaterialName, materials) != nullptr)
        {
            for(auto& currentTexture : FindUnorderdMap(currentMaterialName, materials)->textureObjects)
            {
                currentTexture.DestroyTexture();
            }
            materials.erase(currentMaterialName);
        }
    }
    Renderer::RemoveAllocatedBuffer("material buffer", false);
}
