#pragma once 

#include "../../Vulkan/Include/vk_renderer.h"
#include "../../Window/Include/WindowHandler.h"


namespace RendererCore
{
    void init_renderer();
    void update_renderer();
    void renderer_events(SDL_Event& ev);
    void cleanup_renderer();
};