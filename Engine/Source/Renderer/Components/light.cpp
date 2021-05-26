#include "Include/light.h"

#include "Include/renderer.h"
#include "glm/fwd.hpp"

void LightManager::Init()
{
    Renderer::CreateShaderUniformBuffer("light buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPULightData));
    

    Renderer::WriteShaderUniform("camera data", "per frame layout",1,false,"light buffer");
}

void LightManager::Update()
{
    GPULightData lightData{};
    lightData.pLights[0].position = glm::vec4(glm::vec3(-10.0f,  10.0f, 10.0f),0.0f);
    lightData.pLights[1].position = glm::vec4(glm::vec3(10.0f,  10.0f, 10.0f),0.0f);
    lightData.pLights[2].position = glm::vec4(glm::vec3(-10.0f, -10.0f, 10.0f), 0.0f);
    lightData.pLights[3].position = glm::vec4(glm::vec3(10.0f, -10.0f, 10.0f),0.0f);

    lightData.camPos = glm::vec4(GetCamPos(), 0.0f);

    Renderer::UploadSingleUniformDataToShader("light buffer", lightData, false);
}


void LightManager::Destroy()
{
    Renderer::RemoveAllocatedBuffer("light buffer", false);
};