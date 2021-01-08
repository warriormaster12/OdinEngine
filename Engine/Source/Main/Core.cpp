#include "Include/Core.h"
#include "../Vulkan/Include/vk_renderer.h"
#include "../Window/Include/WindowHandler.h"
#include "../Logger/Include/Logger.h"

#include "../ECS/Component/TestComponent.h"
#include "../ECS/System/Include/TestSystem.h"
#include "../ECS/Include/Coordinator.h"

bool _isInitialized{ false };
Coordinator gCoordinator;
VulkanRenderer renderer;
WindowHandler _windowHandler;
void Core::coreInit()
{
    Logger::init();
	//Test of an entity system
	gCoordinator.Init();
	gCoordinator.RegisterComponent<Test>();
	auto testSystem = gCoordinator.RegisterSystem<TestSystem>();
	{
		Signature signature;
		gCoordinator.SetSystemSignature<TestSystem>(signature);
	}
	testSystem->Init();
	
    _windowHandler.createWindow(1920, 1080);
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