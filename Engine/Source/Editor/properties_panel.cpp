#include "Include/properties_panel.h"

#include "imgui.h"
#include "logger.h"
#include "mesh_component.h"
#include "transform_component.h"
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

        if(ImGui::MenuItem("Transform3D"))
        {
            entity.AddComponent(std::make_unique<Transform3D>(), "Transform3D");
        }
        ImGui::EndMenu();
    }
    components = entity.GetComponents();
    
    if(components.size() > 0)
    {
        for(auto& currentComponent : components)
        {
            if(currentComponent == "Transform3D")
            {
                ImGui::Text("Translation");
                static glm::vec3 translation;
                static glm::vec3 lastTranslation;
                ImGui::InputFloat("x", &translation.x);
                ImGui::InputFloat("y", &translation.y);
                ImGui::InputFloat("z", &translation.z);
                if(translation != lastTranslation)
                {
                    static_cast<Transform3D&>(entity.GetComponent(currentComponent)).UpdateTranslation(translation);
                    lastTranslation = translation;
                }
            }
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
                            for(auto& cComponent : components)
                            {
                                if(cComponent == "Transform3D")
                                {
                                    meshComponent.AddMesh(ProjectManager::GetMesh(currentMesh), "", &static_cast<Transform3D&>(entity.GetComponent(cComponent)));
                                    
                                }
                                else {
                                    meshComponent.AddMesh(ProjectManager::GetMesh(currentMesh));
                                }
                            }
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