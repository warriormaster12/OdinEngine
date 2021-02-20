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
    void LoadTextures();
    void LoadMaterials();
    void LoadRenderables();

    void CreateMaterial(const std::string& name);
    Material* GetMaterial(const std::string& name);
    Mesh* GetMesh(const std::string& name);

    WindowHandler GetWindowHandler();

};