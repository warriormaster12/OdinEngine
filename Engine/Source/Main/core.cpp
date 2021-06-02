#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "camera.h"

#include "function_queuer.h"

#include "pipeline_manager.h"
#include "geometry_pipeline.h"
#include "editor_pipeline.h"


bool isInitialized{ false };

Camera camera;

FunctionQueuer additionalDeletion;


auto start = std::chrono::system_clock::now();
auto end = std::chrono::system_clock::now();
float deltaTime;
float timer;


void Core::CoreInit()
{
    Logger::Init();

    
	
	windowHandler.CreateWindow(1920,1080);
    Renderer::InitRenderer(BACKEND_VULKAN);
    Renderer::CreateRenderPass(RENDERPASS_MAIN);
    Renderer::CreateFramebuffer(FRAMEBUFFER_MAIN);

    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1);
    Renderer::CreateShaderUniformLayout("per frame layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC,SHADER_STAGE_FRAGMENT_BIT, 1);
    Renderer::CreateShaderUniformLayout("per object layout");

    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 0, 5);
    Renderer::CreateShaderUniformLayout("texture data layout");

    Renderer::CreateShaderUniformBuffer("camera buffer", true, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUCameraData));
    Renderer::CreateShaderUniformBuffer("camera buffer cube", true, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUCameraData));

    
    PipelineManager::AddRendererPipeline(std::make_unique<GeometryPipeline>());
    Renderer::WriteShaderUniform("camera data", "per frame layout",0,true,"camera buffer");
    PipelineManager::AddRendererPipeline(std::make_unique<EditorPipeline>());

    
    


    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);

    //everything went fine
    isInitialized = true;
}

void Core::CoreUpdate()
{
	//main loop
	while (!windowHandler.WindowShouldClose())
	{
		glfwPollEvents();

        end = std::chrono::system_clock::now();
        using ms = std::chrono::duration<float, std::milli>;
        deltaTime = std::chrono::duration_cast<ms>(end - start).count();
        start = std::chrono::system_clock::now();

        timer += deltaTime * 0.001;

		//RendererCore::RendererEvents();
		Renderer::UpdateRenderer({0.0f, 0.0f, 0.0f, 1.0f}, [=]()
        {
            camera.UpdateCamera(deltaTime);
            PipelineManager::UpdateRendererPipelines();
            
        });
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        additionalDeletion.PushFunction([=](){
            Renderer::RemoveShaderUniformLayout("texture data layout");
            Renderer::RemoveShaderUniformLayout("per frame layout");
            Renderer::RemoveShaderUniformLayout("per object layout");

            
            PipelineManager::DestroyRendererPipelines();
            Renderer::RemoveAllocatedBuffer("camera buffer", true);
            Renderer::RemoveAllocatedBuffer("camera buffer cube", true);
            
        });
        Renderer::CleanUpRenderer(&additionalDeletion);
		windowHandler.DestroyWindow();
    }
}