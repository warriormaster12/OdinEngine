#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include "../../Window/Include/WindowHandler.h"
#include <iostream>

#include "GameViewport.h"

class Editor
{
public:
    static void showMainWindow(Resolution& _resolution);

private:
    static void setupDockSpace();
    
};