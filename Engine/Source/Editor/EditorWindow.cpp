#include "Include/EditorWindow.h"
#include "Include/MaterialProperties.h"


unsigned int width, height;

void Editor::showMainWindow(VulkanRenderer& renderer)
{
    width = renderer.GetWidth();
    height = renderer.GetHeight();
    ImGui::ShowDemoWindow();
    MaterialProperties::CreateWindow("texturedmesh2");
}
