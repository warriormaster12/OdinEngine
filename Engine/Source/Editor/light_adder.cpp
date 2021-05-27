#include "Include/light_adder.h"

#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "imgui.h"

#include "logger.h"
#include "light.h"




void LightAdder::ShowLightAdderWindow()
{
    static bool isActive = false;
    static ImVec4 lightColor;
    static glm::vec3 translation;
    ImGui::Begin("Light adder", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::ColorEdit4("color", (float*)&lightColor);
    ImGui::Text("Translation");
    ImGui::InputFloat("x", &translation.x);
    ImGui::InputFloat("y", &translation.y);
    ImGui::InputFloat("z", &translation.z);
    if(ImGui::Button("Create"))
    {
        LightManager::AddLight(translation, glm::vec3(lightColor.x, lightColor.y, lightColor.z));
    }
    ImGui::End();
}