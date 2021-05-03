#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "materialManager.h"

#include "function_queuer.h"

bool isInitialized{ false };

Mesh mesh;
Mesh mesh2;

Camera camera;

FunctionQueuer additionalDeletion;

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

    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle camera layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle object layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER,SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 1, 2);
    Renderer::CreateShaderUniformLayout("triangle color layout");
    
    ShaderDescriptions descriptionInfo;
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRGB32,offsetof(Vertex, color)},
        {SRG32, offsetof(Vertex, uv)}
    };
    descriptionInfo.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;
    
    Renderer::CreateShader({"EngineAssets/Shaders/triangleShader.frag", "EngineAssets/Shaders/triangleShader.vert"}, "triangle shader", {"triangle camera layout", "triangle object layout","triangle color layout"},&descriptionInfo);

    Renderer::CreateShader({"EngineAssets/Shaders/triangleShader.frag", "EngineAssets/Shaders/triangleShader.vert"}, "triangle shader2", {"triangle camera layout", "triangle object layout","triangle color layout"},&descriptionInfo);
    Renderer::CreateSampler("default sampler", FILTER_NEAREST, SAMPLER_ADDRESS_MODE_REPEAT);

    MaterialManager::CreateMaterial("main mat");
    MaterialManager::CreateMaterial("floor");
    MaterialManager::GetMaterial("main mat").textures = {"EngineAssets/Textures/ExplosionBarrel Diffuse.png", "EngineAssets/Textures/ExplosionBarrel Emission.png"};
    MaterialManager::GetMaterial("floor").textures[0] = "EngineAssets/Textures/viking_room.png";
    MaterialManager::UpdateTextures("main mat");
    MaterialManager::UpdateTextures("floor");

    Renderer::CreateShaderUniformBuffer("camera buffer", true, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUCameraData));
    const int MAX_OBJECTS = 10000;
    Renderer::CreateShaderUniformBuffer("mesh buffer", true, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPUObjectData) * MAX_OBJECTS);

    Renderer::WriteShaderUniform("camera data", "triangle camera layout",0,true,"camera buffer");
    Renderer::WriteShaderUniform("object data", "triangle object layout",0,true,"mesh buffer");
    
    

    Renderer::RemoveShaderUniformLayout("triangle color layout");
    Renderer::RemoveShaderUniformLayout("triangle camera layout");
    Renderer::RemoveShaderUniformLayout("triangle object layout");


    mesh.LoadFromObj("EngineAssets/Meshes/Barrel.obj");
    mesh2.LoadFromObj("EngineAssets/Meshes/Floor.obj");

    mesh.CreateMesh();
    mesh2.CreateMesh();

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

            
            std::vector<GPUObjectData> objectData;
            objectData.resize(2);
            for(int i = 0; i < objectData.size(); i++)
            {
                objectData[i].modelMatrix = glm::rotate(glm::mat4(1.0f), timer * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            Renderer::UploadVectorUniformDataToShader("mesh buffer",objectData, true);

           
            triangleData.color = glm::vec4(1.0f);
            
            
            
            
            Renderer::BindShader("triangle shader");
            
            Renderer::BindUniforms("camera data", 0, true);
            Renderer::BindUniforms("object data", 1,true);
            MaterialManager::BindMaterial("main mat");
            Renderer::BindVertexBuffer(mesh.vertexBuffer);
            Renderer::BindIndexBuffer(mesh.indexBuffer);
            Renderer::DrawIndexed(mesh.indices);

            MaterialManager::BindMaterial("floor");
            Renderer::BindVertexBuffer(mesh2.vertexBuffer);
            Renderer::BindIndexBuffer(mesh2.indexBuffer);
            Renderer::DrawIndexed(mesh2.indices);
        });
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        additionalDeletion.PushFunction([=](){
            MaterialManager::DeleteMaterial("main mat");
            MaterialManager::DeleteMaterial("floor");
            Renderer::DestroySampler("default sampler");
            mesh.DestroyMesh();
            mesh2.DestroyMesh();
            Renderer::RemoveAllocatedBuffer("mesh buffer", true);
            Renderer::RemoveAllocatedBuffer("camera buffer", true);
            
        });
        Renderer::CleanUpRenderer(&additionalDeletion);
		windowHandler.DestroyWindow();
    }
}