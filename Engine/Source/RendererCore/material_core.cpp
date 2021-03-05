#include "Include/material_core.h"
#include "vk_renderer.h"

namespace RendererCore
{
    VulkanRenderer* p_renderer;
    void CreateMaterial(const std::string& name, VulkanRenderer& vkRenderer)
    {
        p_renderer = &vkRenderer;
        vkRenderer.CreateMaterial(&p_renderer->GetMaterial("defaultMat")->materialPass, name);
    }

    Material* GetMaterial(const std::string& name)
    {
        return p_renderer->GetMaterial(name);
    }
}