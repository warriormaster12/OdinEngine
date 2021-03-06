#include "Include/core.h"
#include "renderer_core.h"
#include "window_handler.h"
#include "logger.h"


bool isInitialized{ false };

void Core::CoreInit()
{
    Logger::Init();
	
	RendererCore::InitRenderer();

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