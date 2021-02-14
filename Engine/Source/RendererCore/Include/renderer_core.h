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
    void CreateMaterial(const std::string& name);
    Mesh* GetMesh(const std::string& name);

    void InitScene();


    WindowHandler GetWindowHandler();

};