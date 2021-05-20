#include "Include/material_editor.h"

#include "glm/fwd.hpp"
#include "imgui.h"

#include "logger.h"
#include "materialManager.h"

char materialNameBuffer[64];
char albedoBuffer[64];
char emissionBuffer[64];

bool materialCreated = false;

ImVec4 color;

void MaterialEditor::ShowMaterialWindow()
{
    static bool isActive = false;
    ImGui::Begin("Material Editor", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("Material Name", materialNameBuffer, IM_ARRAYSIZE(materialNameBuffer));
    ImGui::InputText("Albedo texture", albedoBuffer, IM_ARRAYSIZE(albedoBuffer));
    ImGui::InputText("Emission texture", emissionBuffer, IM_ARRAYSIZE(emissionBuffer));
    if(ImGui::Button("Create"))
    {
        MaterialManager::CreateMaterial(materialNameBuffer);
        std::string albedo = albedoBuffer;
        std::string emission = emissionBuffer;
        if(albedo != "" || emission != "")
        {
            MaterialManager::GetMaterial(materialNameBuffer).SetTextures({albedoBuffer, emissionBuffer});
            MaterialManager::AddTextures(materialNameBuffer);
        }
        materialCreated = true;
    }
    ImGui::End();
}