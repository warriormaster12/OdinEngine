#include "Include/light.h"

#include "Include/renderer.h"
#include "glm/fwd.hpp"



std::vector<PointLight> pLights;
int lightCount = 0;

void LightManager::Init()
{
    Renderer::CreateShaderUniformBuffer("light buffer", false, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPULightData));
    

    Renderer::WriteShaderUniform("camera data", "per frame layout",1,false,"light buffer");
}

void LightManager::AddLight(const glm::vec3& position, const glm::vec3& color)
{
    lightCount ++;
    pLights.resize(lightCount);
    pLights[lightCount -1].position = glm::vec4(position, 0.0f);
    pLights[lightCount -1].color = glm::vec4(color, 0.0f);
}

void LightManager::Update()
{
    GPULightData lightData{};
    lightData.camPos = glm::vec4(GetCamPos(), 0.0f);
    if(pLights.size() > 0)
    {
        lightData.lightCount = glm::vec4(lightCount);
        for(int i = 0; i < pLights.size(); i++)
        {
            lightData.pLights[i].position = pLights[i].position;
            lightData.pLights[i].color = pLights[i].color * 300.0f;
        }
    }

    Renderer::UploadSingleUniformDataToShader("light buffer", lightData, false);
}


void LightManager::Destroy()
{
    Renderer::RemoveAllocatedBuffer("light buffer", false);
};