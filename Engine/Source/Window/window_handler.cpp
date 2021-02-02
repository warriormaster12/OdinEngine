#include "Include/window_handler.h"
#include "../Logger/Include/logger.h"

void WindowHandler::CreateWindow(uint32_t width, uint32_t height)
{
    // We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    resolution.width = width;
    resolution.height = height;
	
	window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		resolution.width,
		resolution.height,
		window_flags
	);

    ENGINE_CORE_INFO("window intialized");
}

void WindowHandler::DestroyWindow()
{
    SDL_DestroyWindow(window);
    ENGINE_CORE_INFO("window destroyed");
}
