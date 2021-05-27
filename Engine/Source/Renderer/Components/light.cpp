#include "Include/light.h"

#include "Include/renderer.h"



std::vector<PointLight> pLights;

void LightManager::Init()
{
    pLights.resize(4);
    Renderer::CreateShaderUniformBuffer("light buffer", false, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPULightData));
    

    Renderer::WriteShaderUniform("camera data", "per frame layout",1,false,"light buffer");
}

void LightManager::Update()
{
    GPULightData lightData{};
    lightData.lightCount = glm::vec4(4);
    lightData.camPos = glm::vec4(GetCamPos(), 0.0f);
    pLights[0].position = glm::vec4(glm::vec3(-10.0f,  10.0f, 10.0f),0.0f);
    pLights[1].position = glm::vec4(glm::vec3(10.0f,  10.0f, 10.0f),0.0f);
    pLights[2].position = glm::vec4(glm::vec3(-10.0f, -10.0f, 10.0f), 0.0f);
    pLights[3].position = glm::vec4(glm::vec3(10.0f, -10.0f, 10.0f),0.0f);
    for(int i = 0; i < pLights.size(); i++)
    {
        lightData.pLights[i].position = pLights[i].position;
    }

    Renderer::UploadSingleUniformDataToShader("light buffer", lightData, false);
}


void LightManager::Destroy()
{
    Renderer::RemoveAllocatedBuffer("light buffer", false);
};