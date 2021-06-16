#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

#include "material_manager.h"
#include "render_object.h"
#include "light.h"

#include "camera.h"

#include "function_queuer.h"

#include "pipeline_manager.h"
#include "geometry_pipeline.h"
#include "editor_pipeline.h"
#include "debug_pipeline.h"
#include "composition_pipeline.h"
#include "statistics.h"
#include <memory>

//temp 
#include "scene_manager.h"
#include "mesh_component.h"


bool isInitialized{ false };

FunctionQueuer additionalDeletion;



void Core::CoreInit()
{
    Logger::Init();

    SceneManager::CreateScene("scene");
    auto& scene1 = SceneManager::GetScene("scene");
    scene1.AddEntity("entity");
    scene1.AddEntity("entity2");

    scene1.GetEntity("entity")->AddComponent(std::make_unique<MeshComponent>(), "Mesh");
    
	windowHandler.CreateWindow(1920,1080);
    Renderer::InitRenderer(BACKEND_VULKAN);
    Renderer::CreateFramebuffer(FRAMEBUFFER_MAIN);

    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1);
    Renderer::CreateShaderUniformLayout("per frame layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC,SHADER_STAGE_FRAGMENT_BIT, 1);
    Renderer::CreateShaderUniformLayout("per object layout");

    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 0, 5);
    Renderer::CreateShaderUniformLayout("texture data layout");

    ObjectManager::Init();
    MaterialManager::Init();
    LightManager::Init();

    
    CameraManager::Init();

    CameraManager::AddCamera("camera");
    CameraManager::GetCamera("camera").SetIsActive(true);

    CameraManager::AddCamera("camera2");
    CameraManager::GetCamera("camera2").SetIsActive(false);

    PipelineManager::AddRendererPipeline(std::make_unique<CompositionPipeline>());
    PipelineManager::AddRendererPipeline(std::make_unique<GeometryPipeline>());
    PipelineManager::AddRendererPipeline(std::make_unique<DebugPipeline>());
    PipelineManager::AddRendererPipeline(std::make_unique<EditorPipeline>());
    


    CameraManager::GetCamera("camera").position = glm::vec3(0.0f, 0.0f, 5.0f);
    CameraManager::GetCamera("camera2").position = glm::vec3(0.0f, 5.0f, 5.0f);

    MaterialManager::CreateMaterial("Test");

    std::vector<std::string> names;
    scene1.GetEntity("entity")->GetComponents(&names);
    for(int i = 0; i < names.size(); i++)
    {
        ENGINE_CORE_TRACE(names[i]);
    }
    static_cast<MeshComponent&>(scene1.GetEntity("entity")->GetComponent("Mesh")).AddMesh("EngineAssets/Meshes/Barrel.obj");

    //everything went fine
    isInitialized = true;
}

void Core::CoreUpdate()
{
	//main loop
	while (!windowHandler.WindowShouldClose())
	{
		glfwPollEvents();
        Statistics::Start();
		//RendererCore::RendererEvents();
        if(windowHandler.GetKInput(GLFW_KEY_J) == GLFW_PRESS)
        {
            CameraManager::AddCamera("camera");
            CameraManager::GetCamera("camera").SetIsActive(false);

            CameraManager::AddCamera("camera2");
            CameraManager::GetCamera("camera2").SetIsActive(true);

        }
        if(windowHandler.GetKInput(GLFW_KEY_K) == GLFW_PRESS)
        {
            CameraManager::AddCamera("camera");
            CameraManager::GetCamera("camera").SetIsActive(true);

            CameraManager::AddCamera("camera2");
            CameraManager::GetCamera("camera2").SetIsActive(false);
        }
        CameraManager::Render();
        PipelineManager::UpdateRendererPipelines();

		Renderer::UpdateRenderer();
        Statistics::End();
        CameraManager::UpdateInput(Statistics::GetDeltaTime());
        auto& scene1 = SceneManager::GetScene("scene");
        scene1.UpdateEntities(Statistics::GetDeltaTime());
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        additionalDeletion.PushFunction([=](){            
            PipelineManager::DestroyRendererPipelines();
            CameraManager::Destroy();
        });
        Renderer::CleanUpRenderer(&additionalDeletion);
		windowHandler.DestroyWindow();
    }
}