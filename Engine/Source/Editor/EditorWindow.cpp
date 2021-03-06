#include "Include/EditorWindow.h"


unsigned int width, height;
std::array <float, 4> imColor;
void Editor::ObjectEditor()
{
    ImGui::Begin("Object Editor");
    ImGui::ColorPicker4("albedo", imColor.data());
    ImGui::End();
}
std::array<float, 4> Editor::GetColor()
{
    return imColor;
}
void Editor::showMainWindow(VulkanRenderer& renderer)
{
    width = renderer.GetWidth();
    height = renderer.GetHeight();
    ObjectEditor();
}
