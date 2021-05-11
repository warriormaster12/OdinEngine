#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "renderobject.h"
#include "materialManager.h"
#include "camera.h"

#include "function_queuer.h"

bool isInitialized{ false };

Mesh mesh;
Mesh mesh2;

RenderObject barrelObj;
RenderObject barrelObj2;
RenderObject floorObj;

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
    Renderer::CreateShaderUniformLayout("triangle camera layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_STORAGE_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("triangle object layout");
    
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER,SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 1, 2);
    Renderer::CreateShaderUniformLayout("material data layout");
    
    ShaderDescriptions descriptionInfo;
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRGB32,offsetof(Vertex, color)},
        {SRG32, offsetof(Vertex, uv)}
    };
    descriptionInfo.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;
    
    Renderer::CreateShader({"EngineAssets/Shaders/defaultTexturedWorld.frag", "EngineAssets/Shaders/defaultTexturedWorld.vert"}, "default textured world", {"triangle camera layout", "triangle object layout","material data layout"},&descriptionInfo);

    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
    };

    Renderer::CreateShader({"EngineAssets/Shaders/defaultWorld.frag", "EngineAssets/Shaders/defaultWorld.vert"}, "default world", {"triangle camera layout", "triangle object layout","material data layout"},&descriptionInfo);
    Renderer::CreateSampler("default sampler", FILTER_NEAREST, SAMPLER_ADDRESS_MODE_REPEAT);

    MaterialManager::CreateMaterial("main mat");
    MaterialManager::CreateMaterial("floor");
    MaterialManager::GetMaterial("floor").repeateCount = 2;
    MaterialManager::GetMaterial("main mat").textures = {"EngineAssets/Textures/ExplosionBarrel Diffuse.png", "EngineAssets/Textures/ExplosionBarrel Emission.png"};
    //MaterialManager::GetMaterial("floor").textures = {"EngineAssets/Textures/wall.jpg", ""};
    MaterialManager::GetMaterial("floor").color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    MaterialManager::AddTextures("main mat");
    //MaterialManager::AddTextures("floor");

    Renderer::CreateShaderUniformBuffer("camera buffer", true, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUCameraData));
    

    Renderer::WriteShaderUniform("camera data", "triangle camera layout",0,true,"camera buffer");
    
    
    

    


    ObjectManager::Init();

    mesh.LoadFromObj("EngineAssets/Meshes/Barrel.obj");
    mesh2.LoadFromObj("EngineAssets/Meshes/Floor.obj");

    mesh.CreateMesh();
    mesh2.CreateMesh();

    barrelObj.p_mesh = &mesh;
    barrelObj.material = "main mat";
    barrelObj.transformMatrix = glm::translate(glm::vec3( 0,5,0 ));

    barrelObj2.p_mesh = &mesh;
    barrelObj2.material = "main mat";
    barrelObj2.transformMatrix = glm::translate(glm::vec3( 1,5,3 ));


    floorObj.p_mesh = &mesh2;
    floorObj.material = "floor";
    floorObj.transformMatrix = glm::translate(glm::vec3( 6,-5,2 ));

    ObjectManager::PushObjectToQueue(barrelObj);
    ObjectManager::PushObjectToQueue(barrelObj2);
    ObjectManager::PushObjectToQueue(floorObj);

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
            
            ObjectManager::RenderObjects();
        });
    }
}

void Core::CoreCleanup()
{
    if (isInitialized)
    {
        additionalDeletion.PushFunction([=](){
            Renderer::RemoveShaderUniformLayout("material data layout");
            Renderer::RemoveShaderUniformLayout("triangle camera layout");
            Renderer::RemoveShaderUniformLayout("triangle object layout");

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