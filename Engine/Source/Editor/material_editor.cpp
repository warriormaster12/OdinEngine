#include "Include/material_editor.h"

#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "imgui.h"

#include "logger.h"
#include "material_manager.h"

#include <vector>

bool materialCreated = false;

ImVec4 color;

void MaterialEditor::ShowMaterialWindow()
{
    static bool isActive = false;
    static char materialNameBuffer[64];
    static char albedoBuffer[64];
    static char metallicBuffer[64];
    static char roughnessBuffer[64];
    static char aoBuffer[64];
    static char normalBuffer[64];
    static std::vector<std::string> textures;
    textures.resize(5);
    ImGui::Begin("Material Editor", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("Material Name", materialNameBuffer, IM_ARRAYSIZE(materialNameBuffer));
    ImGui::InputText("Albedo texture", albedoBuffer, IM_ARRAYSIZE(albedoBuffer));
    ImGui::InputText("Metallic texture", metallicBuffer, IM_ARRAYSIZE(metallicBuffer));
    ImGui::InputText("Roughness texture", roughnessBuffer, IM_ARRAYSIZE(roughnessBuffer));
    ImGui::InputText("Ao texture", aoBuffer, IM_ARRAYSIZE(aoBuffer));
    ImGui::InputText("Normal texture", normalBuffer, IM_ARRAYSIZE(normalBuffer));
    ImGui::ColorEdit4("albedo", (float*)&color);

    for(int i = 0; i< textures.size(); i++)
    {
        textures[i] = "EngineAssets/Textures/";
    }
    
    textures[0] += albedoBuffer;
    textures[1] += metallicBuffer;
    textures[2] += roughnessBuffer;
    textures[3] += aoBuffer ;
    textures[4] += normalBuffer;
    if(ImGui::Button("Create"))
    {
         
        MaterialManager::CreateMaterial(materialNameBuffer);
        MaterialManager::GetMaterial(materialNameBuffer).SetColor(glm::vec4(color.x, color.y, color.z, 1.0f));

        for(int i = 0; i < textures.size(); i++)
        {
            if(textures[i] != "EngineAssets/Textures/")
            {
                MaterialManager::GetMaterial(materialNameBuffer).textureCheck.textures[i] = 1;
            }
            else {
                MaterialManager::GetMaterial(materialNameBuffer).textureCheck.textures[i] = 0;
            }
        }
        if(textures.size() != 0)
        {
            MaterialManager::GetMaterial(materialNameBuffer).SetTextures({textures});
            
            MaterialManager::AddTextures(materialNameBuffer);
        }
        materialCreated = true;
    }
    if(ImGui::Button("Delete") && materialCreated == true)
    {
        MaterialManager::DeleteMaterial(materialNameBuffer);
    }
    ImGui::End();
}