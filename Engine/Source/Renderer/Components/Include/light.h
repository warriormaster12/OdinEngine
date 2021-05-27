#pragma once  

#include <iostream>
#include <glm/glm.hpp>
#include <vector>


class LightManager
{
public:
    static void Init();
    static void AddLight(const glm::vec3& position, const glm::vec3& color);
    static void Update();
    static void SetCamPos(glm::vec3& other) {camPos = other;}
    static glm::vec3& GetCamPos() {return camPos;}
    static void Destroy();
private:
   static inline glm::vec3 camPos;
};

struct PointLight
{
    glm::vec4 position = glm::vec4(0.0f); //vec3
    glm::vec4 color = glm::vec4(0.0f); //vec3
};


struct GPULightData
{
    glm::vec4 lightCount = glm::vec4(1); //int
    glm::vec4 camPos; //vec3
    PointLight pLights[32];
};