#pragma once 

#include "../../Third-Party/imgui/Include/imgui.h"
#include "../../Third-Party/imgui/Include/imgui_impl_glfw.h"
#include "../../Third-Party/imgui/Include/imgui_impl_vulkan.h"
#include "../../Vulkan/Include/vk_renderer.h"
#include "EditorWindow.h"

namespace imgui_layer
{
    void InitImguiLayer(VulkanRenderer& renderer, bool showWindow = true);
    void UpdateUi();
}