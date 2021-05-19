#include "Include/materialManager.h"
#include "Include/renderer.h"
#include "glm/fwd.hpp"
#include "logger.h"
#include "unordered_finder.h"

#include <unordered_map>

std::unordered_map<std::string, Material> materials;

std::vector<std::string> materialNameList;

struct GPUMaterialData
{
    glm::vec4 color;
    glm::vec4 repeateCount;
}materialData;

void MaterialManager::CreateMaterial(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    materials[materialName];
    Renderer::CreateShaderUniformBuffer(materialName + " material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUMaterialData));

    Renderer::WriteShaderUniform(materialName, "material data layout",0,false, materialName + " material buffer");

    
    FindUnorderdMap(materialName, materials)->SetColor(glm::vec4(1.0));
    FindUnorderdMap(materialName, materials)->SetRepeateCount(1);

    materialNameList.push_back(materialName);
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
    if(FindUnorderdMap(materialName, materials)->GetTextures().size() != 0)
    {
        Renderer::BindShader("default textured world");
    }
    else {
        Renderer::BindShader("default world");
    }
    
    
    
    if(FindUnorderdMap(materialName, materials)->isUpdated())
    {
        materialData.color = FindUnorderdMap(materialName, materials)->GetColor();
        materialData.repeateCount = glm::vec4(FindUnorderdMap(materialName, materials)->GetRepeateCount());
        Renderer::UploadSingleUniformDataToShader(materialName + " material buffer",materialData, false);
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
        Renderer::RemoveAllocatedBuffer(materialName + " material buffer", false);
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
            Renderer::RemoveAllocatedBuffer(currentMaterialName + " material buffer", false);
            materials.erase(currentMaterialName);
        }
    }
}
