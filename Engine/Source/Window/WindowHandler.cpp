#include "Include/WindowHandler.h"
#include "../Logger/Include/Logger.h"

void WindowHandler::createWindow(uint32_t width, uint32_t height)
{
    // We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    _resolution.width = width;
    _resolution.height = height;
	
	_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_resolution.width,
		_resolution.height,
		window_flags
	);

    ENGINE_CORE_INFO("window intialized");
}

void WindowHandler::destroyWindow()
{
    SDL_DestroyWindow(_window);
    ENGINE_CORE_INFO("window destroyed");
}
