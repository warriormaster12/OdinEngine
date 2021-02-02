#pragma once 

#include "../../third-party/imgui/Include/imgui.h"
#include "../../third-party/imgui/Include/imgui_impl_sdl.h"
#include "../../third-party/imgui/Include/imgui_impl_vulkan.h"
#include "../../Vulkan/Include/vk_renderer.h"
#include "EditorWindow.h"

namespace imgui_layer
{
    void InitImguiLayer(VulkanRenderer& renderer, bool showWindow = true);
    void UpdateUi();
}