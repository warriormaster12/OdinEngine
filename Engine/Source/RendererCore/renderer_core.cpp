#include "Include/renderer_core.h"

VulkanRenderer vkRenderer;
WindowHandler windowHandler;
auto start = std::chrono::system_clock::now();
auto end = std::chrono::system_clock::now();
float deltaTime;
void RendererCore::InitRenderer()
{
    windowHandler.CreateWindow(1920, 1080);
    vkRenderer.Init(windowHandler);
}

void RendererCore::UpdateRenderer()
{
    end = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsed_seconds = end - start;
    deltaTime = elapsed_seconds.count() * 1000.f;
    start = std::chrono::system_clock::now();
   
    vkRenderer.camera.UpdateCamera(deltaTime);
    vkRenderer.BeginDraw();

    vkRenderer.DrawObjects(vkRenderer.renderables.data(), vkRenderer.renderables.size());	
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkRenderer.EndDraw();
}

void RendererCore::RendererEvents(SDL_Event& ev)
{
    vkRenderer.camera.ProcessInputEvent(&ev);
    
    if (ev.type == SDL_WINDOWEVENT_RESIZED)
    {
        vkRenderer.FrameBufferResize();
    }
}

void RendererCore::CleanupRenderer()
{
    vkRenderer.CleanUp();
    windowHandler.DestroyWindow();
}