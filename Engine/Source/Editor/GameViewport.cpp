#include "Include/GameViewport.h"


void Viewport::ShowGameViewport(VulkanRenderer& renderer)
{
    bool windowOpen = true;
    ImGui::Begin("Game viewport",&windowOpen ,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 windowSize = getLargestSizeForViewport();
    ImVec2 windowPos = getCenteredPositionForViewport(windowSize);
   
    ImGui::SetCursorPos(windowPos);
    ImTextureID textureId = 0;
    ImGui::Image(textureId, windowSize, ImVec2(-1,0), ImVec2(0,1));
}

ImVec2 Viewport::getLargestSizeForViewport()
{
    ImVec2 windowSize;
    windowSize = ImGui::GetContentRegionAvail();
    windowSize.x -= ImGui::GetScrollX();
    windowSize.y -= ImGui::GetScrollY();

    float aspectWidth = windowSize.x;
    float aspectHeight = aspectWidth / (16.0f/9.0f);
    if (aspectHeight > windowSize.y)
    {
        aspectHeight = windowSize.y;
        aspectWidth = aspectHeight * (16.0f/9.0f);
    }
    return ImVec2(aspectWidth,aspectHeight);
}
ImVec2 Viewport::getCenteredPositionForViewport(ImVec2 aspectSize)
{
    ImVec2 windowSize;
    windowSize = ImGui::GetContentRegionAvail();
    windowSize.x -= ImGui::GetScrollX();
    windowSize.y -= ImGui::GetScrollY();
    float viewportX = (windowSize.x / 2.0f) - (aspectSize.x / 2.0f);
    float viewportY = (windowSize.y / 2.0f) - (aspectSize.y / 2.0f);
    
    return ImVec2(viewportX,viewportY);
}