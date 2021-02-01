#pragma once 

#include "../../Vulkan/Include/vk_renderer.h"
#include "../../Window/Include/WindowHandler.h"


namespace RendererCore
{
    void InitRenderer();
    void UpdateRenderer();
    void RendererEvents(SDL_Event& ev);
    void CleanupRenderer();
};