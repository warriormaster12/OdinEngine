#pragma once 

#include "../../Third-Party/imgui/Include/imgui.h"
#include "../../Window/Include/window_handler.h"
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