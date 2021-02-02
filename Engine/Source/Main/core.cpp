#include "Include/core.h"
#include "../RendererCore/Include/renderer_core.h"
#include "../Window/Include/window_handler.h"
#include "../Logger/Include/logger.h"

#include "../ECS/Component/TestComponent.h"
#include "../ECS/System/Include/TestSystem.h"
#include "../ECS/Include/Coordinator.h"

#include "../Editor/Include/EditorWindow.h"
#include "../Editor/Include/Imgui_layer.h"


bool isInitialized{ false };
Coordinator gCoordinator;
void Core::CoreInit()
{
    Logger::Init();
	//Test of an entity system
	gCoordinator.Init();
	gCoordinator.RegisterComponent<Test>();
	auto testSystem = gCoordinator.RegisterSystem<TestSystem>();
	{
		Signature signature;
		gCoordinator.SetSystemSignature<TestSystem>(signature);
	}
	testSystem->Init();
	
	RendererCore::InitRenderer();
	//imgui_layer::init_imgui_layer(renderer);
    //everything went fine
    isInitialized = true;
}

void Core::CoreUpdate()
{
    SDL_Event e;
	bool bQuit = false;
	
	//main loop
	while (!bQuit)
	{
	
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{

			//ImGui_ImplSDL2_ProcessEvent(&e);
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

			RendererCore::RendererEvents(e);
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