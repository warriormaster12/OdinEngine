#include "Include/scene_graph.h"

#include "imgui.h"

#include "scene_manager.h"
#include <vector>



void SceneGraph::ShowSceneGraphWindow()
{
    static bool isActive = false;
    ImGui::Begin("Scene Graph", &isActive, ImGuiWindowFlags_NoCollapse);

    std::vector<std::string> entities = SceneManager::GetScene("scene").GetEntities();

    int uniqueIndex = 0;
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
    }

    ImGui::End();
}