#include "Include/GameViewport.h"


void Viewport::ShowGameViewport(VulkanRenderer& renderer)
{
    bool window_open = true;
    ImGui::Begin("Game viewport",&window_open ,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 window_size = getLargestSizeForViewport();
    ImVec2 windowPos = getCenteredPositionForViewport(window_size);
   
    ImGui::SetCursorPos(windowPos);
    
    ImTextureID textureId = 0;
    ImGui::Image(textureId, window_size, ImVec2(0,1), ImVec2(1,0));
}

ImVec2 Viewport::getLargestSizeForViewport()
{
    ImVec2 window_size;
    window_size = ImGui::GetContentRegionAvail();
    window_size.x -= ImGui::GetScrollX();
    window_size.y -= ImGui::GetScrollY();

    float aspectWidth = window_size.x;
    float aspectHeight = aspectWidth / (16.0f/9.0f);
    if (aspectHeight > window_size.y)
    {
        aspectHeight = window_size.y;
        aspectWidth = aspectHeight * (16.0f/9.0f);
    }
    return ImVec2(aspectWidth,aspectHeight);
}
ImVec2 Viewport::getCenteredPositionForViewport(ImVec2 aspectSize)
{
    ImVec2 window_size;
    window_size = ImGui::GetContentRegionAvail();
    window_size.x -= ImGui::GetScrollX();
    window_size.y -= ImGui::GetScrollY();
    float viewport_x = (window_size.x / 2.0f) - (aspectSize.x / 2.0f);
    float viewport_y = (window_size.y / 2.0f) - (aspectSize.y / 2.0f);
    
    return ImVec2(viewport_x,viewport_y);
}