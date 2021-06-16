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

    std::vector<std::string> entities = SceneManager::GetScene("scene").GetEntities();

    static std::string clickedEntity;

    static bool showProperties = false;

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