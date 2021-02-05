#pragma once 

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "vk_renderer.h"
#include "EditorWindow.h"

namespace imgui_layer
{
    void InitImguiLayer(VulkanRenderer& renderer, bool showWindow = true);
    void UpdateUi();
}