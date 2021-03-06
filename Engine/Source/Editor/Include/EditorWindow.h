#pragma once 

#include "imgui.h"
#include <iostream>
#include "vk_renderer.h"


namespace Editor
{

    void showMainWindow(VulkanRenderer& renderer);
    void ObjectEditor();
    
    std::array<float, 4> GetColor();
};