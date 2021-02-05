#include "Include/core.h"
#include "renderer_core.h"
#include "window_handler.h"
#include "logger.h"

#include "EditorWindow.h"
#include "Imgui_layer.h"


bool isInitialized{ false };

void Core::CoreInit()
{
    Logger::Init();
	
	RendererCore::InitRenderer();
	//imgui_layer::init_imgui_layer(renderer);

    //everything went fine
    isInitialized = true;
}

void Core::CoreUpdate()
{
	
	//main loop
	while (!RendererCore::GetWindowHandler().WindowShouldClose())
	{
		glfwPollEvents();
		RendererCore::RendererEvents();
		if(RendererCore::GetWindowHandler().GetKInput(GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            RendererCore::GetWindowHandler().WindowClose();
        }
		//imgui_layer::update_ui();
		RendererCore::UpdateRenderer();
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
		RendererCore::CleanupRenderer();
    }
}