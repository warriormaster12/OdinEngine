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

    std::vector<std::string> UpdateTextures(const std::string materialName, std::vector<std::string>& input);
    Mesh* GetMesh(const std::string& name);

    WindowHandler GetWindowHandler();

};