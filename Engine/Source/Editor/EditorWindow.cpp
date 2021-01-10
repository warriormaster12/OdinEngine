#include "Include/EditorWindow.h"

unsigned int width, height;
void Editor::showMainWindow(Resolution& _resolution)
{
    width = _resolution.width;
    height = _resolution.height;
    setupDockSpace();
}

void Editor::setupDockSpace()
{
    bool window_open = true;
    int windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse 
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    if (!ImGui::Begin("main window", &window_open, windowFlags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }
    ImGui::PopStyleVar(2);

    //dockspace
    ImGui::DockSpace(ImGui::GetID("Dockspace"));
}