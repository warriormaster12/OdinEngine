#include "Include/material_editor.h"

#include "glm/fwd.hpp"
#include "imgui.h"

#include "materialManager.h"

char materialNameBuffer[64];

bool materialCreated = false;

ImVec4 color;

void MaterialEditor::ShowMaterialWindow()
{
    static bool isActive = false;
    ImGui::Begin("Material Editor", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("Material Name", materialNameBuffer, IM_ARRAYSIZE(materialNameBuffer));
    if(ImGui::Button("Create"))
    {
       MaterialManager::CreateMaterial(materialNameBuffer);
       materialCreated = true;
    }
    if(materialCreated == true)
    {
        ImGui::ColorEdit4("Albedo", (float*)&color);
        MaterialManager::GetMaterial(materialNameBuffer).SetColor(glm::vec4(color.x, color.y, color.z, color.w));
    }
    ImGui::End();
}