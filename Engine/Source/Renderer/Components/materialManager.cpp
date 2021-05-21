#include "Include/materialManager.h"
#include "Include/renderer.h"
#include "logger.h"
#include "vk_utils.h"
#include "unordered_finder.h"

#include <unordered_map>

std::unordered_map<std::string, Material> materials;

std::vector<std::string> materialNameList;

size_t currentByteOffset = 0;

struct GPUMaterialData
{
    glm::vec4 color;
    glm::vec4 repeateCount;
}materialData;

void MaterialManager::Init()
{
    const int maxMaterial = 30;
    Renderer::CreateShaderUniformBuffer("material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, PadUniformBufferSize(sizeof(GPUMaterialData))* maxMaterial);
    
}

void MaterialManager::CreateMaterial(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    //TODO: figure out a way how to add offset without overwriting existing material
    if(FindUnorderdMap(materialName, materials) == nullptr)
    {
        materials[materialName];
        Renderer::WriteShaderUniform(materialName, "material data layout",0,false,"material buffer",currentByteOffset);
        ENGINE_CORE_TRACE("current offset: {0}", currentByteOffset);
        FindUnorderdMap(materialName, materials)->materialByteOffset = currentByteOffset;
        FindUnorderdMap(materialName, materials)->SetColor(glm::vec4(1.0));
        FindUnorderdMap(materialName, materials)->SetRepeateCount(1);
        //add next offset
        currentByteOffset += PadUniformBufferSize(sizeof(GPUMaterialData));
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
    Renderer::WriteShaderImage(materialName, "material data layout", 1, samplerName, views);
};

Material& MaterialManager::GetMaterial(const std::string& materialName)
{
    return *FindUnorderdMap(materialName, materials);
}

void MaterialManager::BindMaterial(const std::string& materialName)
{
    if(FindUnorderdMap(materialName, materials)->isUpdated())
    {
        materialData.color = FindUnorderdMap(materialName, materials)->GetColor();
        materialData.repeateCount = glm::vec4(FindUnorderdMap(materialName, materials)->GetRepeateCount());
        Renderer::UploadSingleUniformDataToShader("material buffer",materialData, false, FindUnorderdMap(materialName, materials)->materialByteOffset);
        FindUnorderdMap(materialName, materials)->ResetUpdate();   
    }
    Renderer::BindUniforms(materialName,2,false);
}

void MaterialManager::DeleteMaterial(const std::string& materialName)
{
    if(FindUnorderdMap(materialName, materials) != nullptr)
    {
        for(auto& currentTexture : FindUnorderdMap(materialName, materials)->textureObjects)
        {
            currentTexture.DestroyTexture();
        }

        
        if(currentByteOffset != 0)
        {
            if(currentByteOffset > FindUnorderdMap(materialName, materials)->materialByteOffset)
            {
                currentByteOffset = FindUnorderdMap(materialName, materials)->materialByteOffset;
            }
            ENGINE_CORE_TRACE("offset after deletion: {0}", currentByteOffset);
        }
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
