#include "Include/meshAdder.h"

#include "glm/gtx/transform.hpp"
#include "imgui.h"
#include "logger.h"
#include "renderobject.h"

#include <glm/glm.hpp>

#include <string>




char textBuffer[256];
glm::vec3 translation;
void MeshAdder::ShowMeshAdderWindow()
{
    static bool isActive = false;
    static std::string filePath;
    ImGui::Begin("Mesh Adder", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("File Path", textBuffer, IM_ARRAYSIZE(textBuffer));
    
    ImGui::Text("Translation");
    ImGui::InputFloat("x", &translation.x);
    ImGui::InputFloat("y", &translation.y);
    ImGui::InputFloat("z", &translation.z);
    filePath = textBuffer;
    if(ImGui::Button("Apply"))
    {
        RenderObject rObj;
    

        rObj.p_mesh = new Mesh;
        rObj.p_mesh->LoadFromObj(filePath);
        rObj.p_mesh->CreateMesh();
        rObj.material = "main mat";
        rObj.transformMatrix = glm::translate(translation);

        ObjectManager::PushObjectToQueue(rObj);
    }
    //ImGui::InputText("file path", textBuffer, 60);
    ImGui::End();
}