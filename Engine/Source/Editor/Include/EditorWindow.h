#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include <iostream>
#include "../../Vulkan/Include/vk_renderer.h"

#include "GameViewport.h"

class Editor
{
public:
    static void showMainWindow(VulkanRenderer& renderer);

private:
    static void setupDockSpace();
    
};