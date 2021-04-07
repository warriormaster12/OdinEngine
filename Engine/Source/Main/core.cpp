#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "vk_utils.h"


bool isInitialized{ false };
AllocatedBuffer triangleBuffer;

struct TriangleData
{
    glm::vec4 color;
}triangleData;


void UploadData(const TriangleData& data)
{
    UploadSingleData(VkDeviceManager::GetAllocator(), triangleBuffer.allocation, data);
}


void Core::CoreInit()
{
    Logger::Init();
	
	windowHandler.CreateWindow(1920,1080);
    Renderer::InitRenderer(BACKEND_VULKAN);
    Renderer::CreateRenderPass(RENDERPASS_MAIN);
    Renderer::CreateFramebuffer(FRAMEBUFFER_MAIN);

    //TODO: replace Vulkan bit fields with custom one
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle color layout");
    
    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader", "triangle color layout");

    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader2", "triangle color layout");

    Renderer::WriteShaderUniform("triangle color", triangleBuffer, sizeof(TriangleData));

    Renderer::RemoveShaderUniformLayout("triangle color layout");

    //everything went fine
    isInitialized = true;
}

void Core::CoreUpdate()
{
	
	//main loop
	while (!windowHandler.WindowShouldClose())
	{
		glfwPollEvents();
		//RendererCore::RendererEvents();
		if(windowHandler.GetKInput(GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            windowHandler.WindowClose();
        }
		Renderer::UpdateRenderer({0.0f, 0.0f, 0.0f, 1.0f}, [=]()
        {
            if(windowHandler.GetKInput(GLFW_KEY_E) == GLFW_PRESS)
            {
                triangleData.color = glm::vec4(1.0f,0.0f,0.0f, 1.0f);
                Renderer::BindShader("triangle shader");
            }
            else 
            {
                triangleData.color = glm::vec4(1.0f,1.0f,0.0f, 1.0f);
                Renderer::BindShader("triangle shader2");
            }
            UploadData(triangleData);
            
            Renderer::BindUniforms("triangle color", "triangle shader");
            Renderer::Draw();
        });
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        Renderer::RemoveAllocatedBuffer(triangleBuffer);
        Renderer::CleanUpRenderer();
		windowHandler.DestroyWindow();
    }
}