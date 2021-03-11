#include "Include/MaterialProperties.h"
#include "material_core.h"


std::array<float, 4> colors;
namespace MaterialProperties
{
    void CreateWindow(const std::string& materialName)
    {
        glm::vec4 inputColor = glm::vec4(colors[0],colors[1],colors[2], 1.0f);
        ImGui::Begin("Material properties");
        if (ImGui::ColorPicker4("albedo", colors.data()))
        {
            RendererCore::GetMaterial(materialName)->SetAlbedo(inputColor);
        }
        ImGui::End();
        ENGINE_CORE_INFO(RendererCore::GetMaterial(materialName)->isOutdated);
    }
}