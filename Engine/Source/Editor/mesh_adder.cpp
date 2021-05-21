#include "Include/mesh_adder.h"

#include "glm/gtx/transform.hpp"
#include "imgui.h"
#include "logger.h"
#include "renderobject.h"

#include <glm/glm.hpp>

#include <string>




char filePathBuffer[256];
char materialBuffer[64];
glm::vec3 translation;
void MeshAdder::ShowMeshAdderWindow()
{
    static bool isActive = false;
    static std::string filePath;
    ImGui::Begin("Mesh Adder", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("File Path", filePathBuffer, IM_ARRAYSIZE(filePathBuffer));
    ImGui::InputText("Material", materialBuffer, IM_ARRAYSIZE(materialBuffer));
    
    ImGui::Text("Translation");
    ImGui::InputFloat("x", &translation.x);
    ImGui::InputFloat("y", &translation.y);
    ImGui::InputFloat("z", &translation.z);
    filePath = "EngineAssets/Meshes/";
    filePath += filePathBuffer;
    if(ImGui::Button("Apply"))
    {
        RenderObject currentRenderObj;
    

        currentRenderObj.p_mesh->LoadFromObj(filePath);
        currentRenderObj.p_mesh->CreateMesh();
        currentRenderObj.material = materialBuffer;
        currentRenderObj.transformMatrix = glm::translate(translation);

        ObjectManager::PushObjectToQueue(currentRenderObj);
    }
    ImGui::End();
}