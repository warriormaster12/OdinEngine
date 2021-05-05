#include "Include/materialManager.h"
#include "Include/renderer.h"
#include "logger.h"
#include "unordered_finder.h"

#include <unordered_map>

std::unordered_map<std::string, Material> materials;

struct GPUMaterialData
{
    glm::vec4 color;
    glm::vec4 repeateCount;
}materialData;

void MaterialManager::CreateMaterial(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    materials[materialName];
    Renderer::CreateShaderUniformBuffer(materialName + " material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUMaterialData));

    Renderer::WriteShaderUniform(materialName, "triangle color layout",0,false, materialName + " material buffer");

    FindUnorderdMap(materialName, materials)->textureObjects.resize(FindUnorderdMap(materialName, materials)->textures.size());
    std::vector<VkImageView> views;
    for(int i = 0; i < FindUnorderdMap(materialName, materials)->textures.size(); i++)
    {
        FindUnorderdMap(materialName, materials)->textureObjects[i].CreateTexture(FindUnorderdMap(materialName, materials)->textures[i]);
        views.push_back(FindUnorderdMap(materialName, materials)->textureObjects[i].imageView);
    }

    Renderer::WriteShaderImage(materialName, "triangle color layout", 1, samplerName, views);
}

void  MaterialManager::UpdateTextures(const std::string& materialName, const std::string& samplerName /*= "default sampler"*/)
{
    // destroy empty textures first
    for(int i = 0; i < FindUnorderdMap(materialName, materials)->textures.size(); i++)
    {
        auto& currentTexture = FindUnorderdMap(materialName, materials)->textures[i];
    
        FindUnorderdMap(materialName, materials)->textureObjects[i].DestroyTexture();
    }
    std::vector<VkImageView> views;
    for(int i = 0; i < FindUnorderdMap(materialName, materials)->textures.size(); i++)
    {
        FindUnorderdMap(materialName, materials)->textureObjects[i].CreateTexture(FindUnorderdMap(materialName, materials)->textures[i]);
        views.push_back(FindUnorderdMap(materialName, materials)->textureObjects[i].imageView);
    }
    Renderer::WriteShaderImage(materialName, "triangle color layout", 1, samplerName, views);
};

Material& MaterialManager::GetMaterial(const std::string& materialName)
{
    return *FindUnorderdMap(materialName, materials);
}

void MaterialManager::BindMaterial(const std::string& materialName)
{
    materialData.color = FindUnorderdMap(materialName, materials)->color;
    materialData.repeateCount = glm::vec4(FindUnorderdMap(materialName, materials)->repeateCount);
    
    Renderer::UploadSingleUniformDataToShader(materialName + " material buffer",materialData, false);

    Renderer::BindUniforms(materialName,2,false);
}

void MaterialManager::DeleteMaterial(const std::string& materialName)
{
    for(auto& currentTexture : FindUnorderdMap(materialName, materials)->textureObjects)
    {
        currentTexture.DestroyTexture();
    }
    Renderer::RemoveAllocatedBuffer(materialName + " material buffer", false);
}
