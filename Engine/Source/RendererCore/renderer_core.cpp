#include "Include/renderer_core.h"

VulkanRenderer vk_renderer;
WindowHandler _windowHandler;
auto start = std::chrono::system_clock::now();
auto end = std::chrono::system_clock::now();
float deltatime;
void RendererCore::init_renderer()
{
    _windowHandler.createWindow(1920, 1080);
    vk_renderer.init(_windowHandler);
}

void RendererCore::update_renderer()
{
    end = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsed_seconds = end - start;
    deltatime = elapsed_seconds.count() * 1000.f;
    start = std::chrono::system_clock::now();
   
    vk_renderer._camera.update_camera(deltatime);
    vk_renderer.begin_draw();

    vk_renderer.draw_objects(vk_renderer._renderables.data(), vk_renderer._renderables.size());	
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vk_renderer.end_draw();
}

void RendererCore::renderer_events(SDL_Event& ev)
{
    vk_renderer._camera.process_input_event(&ev);
    
    if (ev.type == SDL_WINDOWEVENT_RESIZED)
    {
        vk_renderer.frameBufferResize();
    }
}

void RendererCore::cleanup_renderer()
{
    vk_renderer.cleanup();
    _windowHandler.destroyWindow();
}