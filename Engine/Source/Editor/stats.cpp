#include "Include/stats.h"

#include "logger.h"
#include "statistics.h"

#include "imgui.h"


void Stats::ShowStatsWindow()
{
    static bool isActive = false;
    ImGui::Begin("Stats", &isActive, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("deltaTime: %f", Statistics::GetDeltaTime());
    ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / Statistics::GetFps()), (uint32_t)Statistics::GetFps());

    ImGui::End();
}