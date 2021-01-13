#include "Include/EditorWindow.h"
#include "Include/GameViewport.h"


unsigned int width, height;
void Editor::showMainWindow(VkExtent2D& _contentExtent)
{
    
    width = _contentExtent.width;
    height = _contentExtent.height;
    setupDockSpace();
    ImGui::ShowDemoWindow();
    Viewport::ShowGameViewport();
	ImGui::End();
}

void Editor::setupDockSpace()
{
    bool window_open = true;
    int windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize 
        | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("editor window", &window_open, windowFlags);
    ImGui::PopStyleVar(2);

    //dockspace
    ImGui::DockSpace(ImGui::GetID("Dockspace"));
}