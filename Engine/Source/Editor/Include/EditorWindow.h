#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include <iostream>
#include "../../Vulkan/Include/vk_types.h"

#include "GameViewport.h"

class Editor
{
public:
    static void showMainWindow(VkExtent2D& _contentExtent);

private:
    static void setupDockSpace();
    
};