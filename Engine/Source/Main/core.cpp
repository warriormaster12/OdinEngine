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
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle color layout");
    Renderer::CreateShaderUniformLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle camera layout");
    
    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader", {"triangle color layout", "triangle camera layout"});

    Renderer::CreateShader({".Shaders/triangleShader.frag", ".Shaders/triangleShader.vert"}, "triangle shader2", {"triangle color layout", "triangle camera layout"});

    Renderer::WriteShaderUniform("triangle color", "triangle color layout",triangleBuffer, sizeof(TriangleData));
    Renderer::WriteShaderUniform("camera data", "triangle camera layout",camera.cameraBuffer, sizeof(GPUCameraData));

    Renderer::RemoveShaderUniformLayout("triangle color layout");
    Renderer::RemoveShaderUniformLayout("triangle camera layout");

    mesh.vertices.resize(4);
    //vertex positions
    mesh.vertices[0].position = {-0.5f, -0.5f, 0.0f };
    mesh.vertices[1].position = {0.5f, -0.5f, 0.0f };
    mesh.vertices[2].position = { 0.5f, 0.5f, 0.0f};
    mesh.vertices[3].position = {-0.5f, 0.5f, 0.0f};
    mesh.indices = {0, 1, 2, 2, 3, 0};

    mesh.CreateMesh();

    camera.position = glm::vec3(0.0f, 0.0f, 2.0f);

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
            end = std::chrono::system_clock::now();
            using ms = std::chrono::duration<float, std::milli>;
            deltaTime = std::chrono::duration_cast<ms>(end - start).count();
            start = std::chrono::system_clock::now();

            timer += deltaTime * 0.0001;
            
            camera.UpdateCamera(deltaTime);

            GPUCameraData camData{};
            camData.modelMatrix = glm::rotate(glm::mat4(1.0f), timer * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            camData.viewMatrix = camera.GetViewMatrix();
            camData.projectionMatrix = camera.GetProjectionMatrix();
            Renderer::UploadUniformDataToShader(camData,camera.cameraBuffer);
	
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
            
            Renderer::BindUniforms("triangle color", "triangle shader");
            Renderer::BindUniforms("camera data", "triangle shader", 1);
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
        Renderer::RemoveAllocatedBuffer(triangleBuffer);
        Renderer::RemoveAllocatedBuffer(camera.cameraBuffer);
        mesh.DestroyMesh();
        Renderer::CleanUpRenderer();
		windowHandler.DestroyWindow();
    }
}