#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include "../../Window/Include/WindowHandler.h"
#include "../../Vulkan/Include/vk_renderer.h"
#include <iostream>


class Viewport
{
public:
    static void ShowGameViewport(VulkanRenderer& renderer);
private:
    static ImVec2 getLargestSizeForViewport();
    static ImVec2 getCenteredPositionForViewport(ImVec2 aspectSize);
};