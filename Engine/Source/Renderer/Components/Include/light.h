#pragma once  

#include <iostream>
#include <glm/glm.hpp>


class LightManager
{
public:
    static void Init();
    static void Update();
    static void SetCamPos(glm::vec3& other) {camPos = other;}
    static glm::vec3& GetCamPos() {return camPos;}
    static void Destroy();
private:
   static inline glm::vec3 camPos;
};

struct PointLight
{
    glm::vec4 position; //vec3
    glm::vec4 color = glm::vec4(300.0f); //vec3
};


struct GPULightData
{
    PointLight pLights[4];
    glm::vec4 camPos; //vec3
};