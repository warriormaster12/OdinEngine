#include "Include/meshAdder.h"

#include "glm/gtx/transform.hpp"
#include "imgui.h"
#include "renderobject.h"

#include <glm/glm.hpp>

#include <string>

Mesh mesh3;

RenderObject barrelObj3;

char textBuffer[256];
void MeshAdder::ShowMeshAdderWindow()
{
    static bool isActive = false;
    static std::string filePath;
    ImGui::Begin("Mesh Adder", &isActive, ImGuiWindowFlags_NoCollapse);
    ImGui::InputText("File Path", textBuffer, IM_ARRAYSIZE(textBuffer));
    filePath = textBuffer;
    if(ImGui::Button("Apply"))
    {
        mesh3.LoadFromObj(filePath);
        mesh3.CreateMesh();

        barrelObj3.p_mesh = &mesh3;
        barrelObj3.material = "main mat";
        barrelObj3.transformMatrix = glm::translate(glm::vec3(1.0f, 1.0f, 1.0f));

        ObjectManager::PushObjectToQueue(barrelObj3);
    }
    //ImGui::InputText("file path", textBuffer, 60);
    ImGui::End();
}