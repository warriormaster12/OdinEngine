#include "Include/scene_graph.h"

#include "imgui.h"

#include "logger.h"
#include "scene_manager.h"
#include "Include/properties_panel.h"
#include <vector>



void SceneGraph::ShowSceneGraphWindow()
{
    static bool isActive = false;
    ImGui::Begin("Scene Graph", &isActive, ImGuiWindowFlags_NoCollapse);

    static bool showAddOptions = false;
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
            static std::string name;
            name.resize(32);
            if(ImGui::InputText("entity name", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                SceneManager::GetScene("scene").AddEntity(name);
                name = "";
                showAddOptions = false;
            }
            if(ImGui::Button("Apply"))
            {
                SceneManager::GetScene("scene").AddEntity(name);
                name = "";
                showAddOptions = false;
            }
            ImGui::EndMenu();
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