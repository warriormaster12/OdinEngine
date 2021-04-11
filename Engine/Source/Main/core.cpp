#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "vk_utils.h"
#include "mesh.h"
#include "camera.h"


bool isInitialized{ false };
AllocatedBuffer triangleBuffer;

Mesh mesh;
Camera camera;

struct TriangleData
{
    glm::vec4 color;
}triangleData;



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

    //TODO: replace Vulkan bit fields with custom one
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle camera layout");
    
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle object layout");
    
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle color layout");
    
    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader", {"triangle camera layout", "triangle object layout","triangle color layout"});

    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader2", {"triangle camera layout", "triangle object layout","triangle color layout"});

    //TODO: replace Vulkan bit fields with custom one
    Renderer::WriteShaderUniform("triangle color", "triangle color layout",VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,triangleBuffer, sizeof(TriangleData));
    Renderer::WriteShaderUniform("camera data", "triangle camera layout",VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,camera.cameraBuffer, sizeof(GPUCameraData));
    Renderer::WriteShaderUniform("object data", "triangle object layout",VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,mesh.meshBuffer, sizeof(GPUObjectData));

    Renderer::RemoveShaderUniformLayout("triangle color layout");
    Renderer::RemoveShaderUniformLayout("triangle camera layout");
    Renderer::RemoveShaderUniformLayout("triangle object layout");

    mesh.LoadFromObj("EngineAssets/Meshes/monkey_smooth.obj");

    mesh.CreateMesh();

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
		if(windowHandler.GetKInput(GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            windowHandler.WindowClose();
        }
		Renderer::UpdateRenderer({0.0f, 0.0f, 0.0f, 1.0f}, [=]()
        {
            
            camera.UpdateCamera(deltaTime);

            
            GPUObjectData objectData{};
            objectData.modelMatrix = glm::rotate(glm::mat4(1.0f), timer * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            Renderer::UploadUniformDataToShader(objectData,mesh.meshBuffer);

           
	
            if(windowHandler.GetKInput(GLFW_KEY_G) == GLFW_PRESS)
            {
                triangleData.color = glm::vec4(1.0f,0.0f,0.0f, 1.0f);
                Renderer::BindShader("triangle shader");
            }
            else 
            {
                triangleData.color = glm::vec4(1.0f,1.0f,0.0f, 1.0f);
                Renderer::BindShader("triangle shader2");
            }
            
            
            Renderer::UploadUniformDataToShader(triangleData, triangleBuffer);
            
            
            Renderer::BindUniforms("camera data", "triangle shader", 0);
            Renderer::BindUniforms("object data", "triangle shader", 1);
            Renderer::BindUniforms("triangle color", "triangle shader",2);
            Renderer::BindVertexBuffer(mesh.vertexBuffer);
            Renderer::BindIndexBuffer(mesh.indexBuffer);
            Renderer::DrawIndexed(mesh.indices);
        });
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        Renderer::RemoveAllocatedBuffer(mesh.meshBuffer);
        Renderer::RemoveAllocatedBuffer(triangleBuffer);
        Renderer::RemoveAllocatedBuffer(camera.cameraBuffer);
        mesh.DestroyMesh();
        Renderer::CleanUpRenderer();
		windowHandler.DestroyWindow();
    }
}