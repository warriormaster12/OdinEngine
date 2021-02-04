#pragma once 

#include "../../Vulkan/Include/vk_renderer.h"
#include "../../Window/Include/window_handler.h"


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