#include "Include/EditorWindow.h"
#include "Include/GameViewport.h"


unsigned int width, height;
void Editor::showMainWindow(VulkanRenderer& renderer)
{
    width = renderer.GetWidth();
    height = renderer.GetHeight();
    //setupDockSpace();
    ImGui::ShowDemoWindow();
    //Viewport::ShowGameViewport(renderer);
}

void Editor::setupDockSpace()
{
    bool window_open = true;
    int windowFlags = ImGuiWindowFlags_MenuBar;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize 
        | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("editor window", &window_open, windowFlags);
    ImGui::PopStyleVar(2);
}