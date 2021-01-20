#include "Include/Core.h"
#include "../Vulkan/Include/vk_renderer.h"
#include "../Window/Include/WindowHandler.h"
#include "../Logger/Include/Logger.h"

#include "../ECS/Component/TestComponent.h"
#include "../ECS/System/Include/TestSystem.h"
#include "../ECS/Include/Coordinator.h"

#include "../Editor/Include/EditorWindow.h"
#include "../Editor/Include/Imgui_layer.h"


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
	imgui_layer::init_imgui_layer(renderer);
    //everything went fine
    _isInitialized = true;
}

void Core::coreUpdate()
{
    SDL_Event e;
	bool bQuit = false;
	auto start = std::chrono::system_clock::now();
	auto end = std::chrono::system_clock::now();
	//main loop
	while (!bQuit)
	{
		end = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed_seconds = end - start;
		auto deltatime = elapsed_seconds.count() * 1000.f;

		start = std::chrono::system_clock::now();

	
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{

			ImGui_ImplSDL2_ProcessEvent(&e);
			renderer._camera.process_input_event(&e);
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
			else if (e.type == SDL_WINDOWEVENT_RESIZED)
			{
				renderer.frameBufferResize();
			}
		}
		imgui_layer::update_ui();
		renderer._camera.update_camera(deltatime);
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