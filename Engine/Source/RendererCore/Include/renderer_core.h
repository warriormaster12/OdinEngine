#pragma once 

#include "vk_renderer.h"
#include "window_handler.h"


namespace RendererCore
{
    void InitRenderer();
    void UpdateRenderer();
    void RendererEvents();
    void CleanupRenderer();

    void InitScene();
    void LoadMeshes();
    void LoadMaterials();
    void LoadRenderables();

    WindowHandler GetWindowHandler();

};