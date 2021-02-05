#pragma once 

#include "vk_renderer.h"
#include "window_handler.h"


namespace RendererCore
{
    void InitRenderer();
    void UpdateRenderer();
    void RendererEvents();
    void CleanupRenderer();

    void LoadMeshes();
    Mesh* GetMesh(const std::string& name);

    void InitScene();


    WindowHandler GetWindowHandler();

};