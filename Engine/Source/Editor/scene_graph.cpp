#include "Include/scene_graph.h"

#include "imgui.h"

#include "logger.h"
#include "scene_manager.h"
#include "Include/properties_panel.h"
#include <memory>
#include <vector>



void SceneGraph::ShowSceneGraphWindow()
{
    static bool isActive = false;
    ImGui::Begin("Scene Graph", &isActive, ImGuiWindowFlags_NoCollapse);

    static bool showAddOptions = false;
    static bool showMesh = false;
    static bool showEmpty = false;
    if(ImGui::Button("Add"))
    {
        if(showAddOptions == false)
        {
            showAddOptions = true;
        }
        else {
            showAddOptions = false;
        }
    }

    if(showAddOptions == true)
    {
        if(ImGui::BeginMenu("select entity type"))
        {
            if(ImGui::MenuItem("empty"))
            {
                showEmpty = true;
            }
            if(ImGui::MenuItem("mesh"))
            {
                showMesh = true;
            }
            if(ImGui::MenuItem("light source"))
            {

            }
            if(ImGui::MenuItem("camera"))
            {
                
            }
            
            ImGui::EndMenu();
        }
        static std::string name;
        name.resize(32);
        if(showEmpty)
        {
            if(ImGui::InputText("entity name", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Apply") && name != "")
            {
                SceneManager::GetScene("scene").AddEntity(name);
                SceneManager::GetScene("scene").GetEntity(name)->AddComponent(std::make_unique<Transform3D>(), "Transform3D");
                name = "";
                showAddOptions = false;
                showEmpty = false;
            }
        }
        if(showMesh)
        {
            if(ImGui::InputText("entity name", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Apply") && name != "")
            {
                SceneManager::GetScene("scene").AddEntity(name);
                SceneManager::GetScene("scene").GetEntity(name)->AddComponent(std::make_unique<Transform3D>(), "Transform3D");
                SceneManager::GetScene("scene").GetEntity(name)->AddComponent(std::make_unique<MeshComponent>(), "Mesh");
                SceneManager::GetScene("scene").GetEntity(name)->AddComponent(std::make_unique<MaterialComponent>(), "Material");
                name = "";
                showAddOptions = false;
                showMesh = false;
            }
        }
    }

    std::vector<std::string> entities = SceneManager::GetScene("scene").GetEntities();

    static std::string clickedEntity;

    static bool showProperties = false;
    static bool showOptions = false;

    int uniqueIndex = 0;
    int clickedNode = -1;
    for(auto& currentEntity : entities)
    {
        ImGui::PushID(uniqueIndex);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool treeNodeOpen = ImGui::TreeNodeEx(currentEntity.c_str(), flags);
        ImGui::PopID();
        if(treeNodeOpen)
        {
            ImGui::TreePop();
        }
        uniqueIndex ++;
        if (ImGui::IsItemClicked())
        {
            clickedEntity = currentEntity;
            showProperties = true;
        }
        if(ImGui::IsItemClicked(1))
        {
            if(showOptions == false)
            {
                clickedEntity = currentEntity;
                showOptions = true;
            }
            else {
                showOptions = false;
            }
        }
    }
    if(showOptions == true)
    {
        static std::string entity = clickedEntity + " options";
        if(ImGui::BeginMenu(entity.c_str()))
        {
            if(ImGui::MenuItem("rename"))
            {

            }
            if(ImGui::MenuItem("delete"))
            {

            }
            ImGui::EndMenu();
        }
    }


    ImGui::End();
    if(showProperties == true)
    {
        if(clickedEntity != "")
        {
            PropertiesPanel::ShowPropertiesPanelWindow(*SceneManager::GetScene("scene").GetEntity(clickedEntity), clickedEntity);
        }
    }
}