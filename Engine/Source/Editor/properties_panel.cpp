#include "Include/properties_panel.h"

#include "imgui.h"
#include "logger.h"
#include "mesh_component.h"
#include "project_manager.h"
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
                    for(auto& currentMesh : ProjectManager::ListMeshes())
                    {
                        if(ImGui::MenuItem(currentMesh.c_str()))
                        {
                            meshComponent.AddMesh(ProjectManager::GetMesh(currentMesh));
                        } 
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndChild();
            }
        }
    }

    ImGui::End();
}