#pragma once 

#include "vk_shaderhandler.h"
#include "vk_pipelinebuilder.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class VulkanRenderer;
//For now this is struct is located outside of the namespace due to some dependencies that need to be resolved first
struct Material {
    VkDescriptorSet materialSet{VK_NULL_HANDLE};
    AllocatedBuffer buffer;
    vkcomponent::ShaderPass materialPass;

    glm::vec4 albedo = glm::vec4(1.0f,1.0f,1.0f,1.0f); // vec4
    float metallic = 0.5f; // float
    float roughness = 0.5f; // float
    float ao = 1.0f; // float

    glm::vec3 emissionColor =  glm::vec3(0.0f,0.0f,0.0f); //vec3
    float emissionPower = 1.0f; // float

    std::vector<std::string> textures = {
        ""
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
    };

    void SetAlbedo(glm::vec4& inputColor);

    bool isOutdated = false;
};
namespace RendererCore
{
    void CreateMaterial(const std::string& name, VulkanRenderer& vkRenderer);
    Material* GetMaterial(const std::string& name);

    std::vector<std::string> UpdateTextures(const std::string materialName, std::vector<std::string>& input);
}