#include "Include/Core.h"
#include "../Vulkan/Include/vk_renderer.h"
#include "../Window/Include/WindowHandler.h"
#include "../Logger/Include/Logger.h"

bool _isInitialized{ false };
VulkanRenderer renderer;
WindowHandler _windowHandler;
void Core::coreInit()
{
    Logger::init();
    _windowHandler.createWindow(1700, 900);
    renderer.init(_windowHandler);
    //everything went fine
    _isInitialized = true;
}

void Core::coreUpdate()
{
    SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//close the window when user alt-f4s or clicks the X button			
			if (e.type == SDL_QUIT)
			{
				bQuit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					bQuit = true;
				}
			}
		}
        renderer.run();
    }
}

void Core::coreCleanup()
{
    if (_isInitialized)
    {
        renderer.cleanup();
        _windowHandler.destroyWindow();
    }
}