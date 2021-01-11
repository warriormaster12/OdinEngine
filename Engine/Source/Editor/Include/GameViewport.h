#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include "../../Window/Include/WindowHandler.h"
#include <iostream>


class Viewport
{
public:
    static void ShowGameViewport();
private:
    static ImVec2 getLargestSizeForViewport();
    static ImVec2 getCenteredPositionForViewport(ImVec2 aspectSize);
};