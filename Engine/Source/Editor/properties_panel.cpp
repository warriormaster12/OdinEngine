#include "Include/properties_panel.h"

#include "imgui.h"
#include "logger.h"
#include "mesh_component.h"
#include <memory>


static std::vector<std::string> components;
void PropertiesPanel::ShowPropertiesPanelWindow(Entity& entity,const std::string& entityName)
{
    static bool isActive = false;
    ImGui::Begin(entityName.c_str(), &isActive, ImGuiWindowFlags_NoCollapse);
    if(ImGui::BeginMenu("Add component"))
    {
        if(ImGui::MenuItem("Mesh"))
        {
            entity.AddComponent(std::make_unique<MeshComponent>(), "Mesh");
            
        }
        ImGui::EndMenu();
    }
    components = entity.GetComponents();
    if(components.size() > 0)
    {
        for(auto& currentComponent : components)
        {
            if(currentComponent == "Mesh")
            {
                static std::string filePath;
                filePath.resize(64);
                ImGui::BeginChild(currentComponent.c_str(), ImVec2(0, 0), true);
                if(ImGui::BeginMenu("Meshes"))
                {
                    auto& meshComponent = static_cast<MeshComponent&>(entity.GetComponent(currentComponent));
                    auto& meshes = static_cast<MeshComponent&>(entity.GetComponent(currentComponent)).GetMeshes();
                    for(int i = 1; i < static_cast<MeshComponent&>(entity.GetComponent(currentComponent)).GetMeshes().size(); i++)
                    {
                        if(ImGui::MenuItem(meshes[i].meshName.c_str()))
                        {
                            meshComponent.ReAddMesh(meshes[i]);
                        } 
                    }
                   
                    
                    ImGui::EndMenu();
                }
                ImGui::InputText("file path", filePath.data(), filePath.size());
                if(ImGui::Button("Apply"))
                {
                    static_cast<MeshComponent&>(entity.GetComponent(currentComponent)).AddMesh(filePath);
                    filePath = "";
                }
                ImGui::EndChild();
            }
        }
    }

    ImGui::End();
}