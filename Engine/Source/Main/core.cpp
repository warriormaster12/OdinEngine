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