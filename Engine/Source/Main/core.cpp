#include "Include/core.h"
#include "window_handler.h"
#include "renderer.h"
#include "logger.h"

//Temporary
#include "vk_utils.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"

#include "function_queuer.h"

bool isInitialized{ false };

Mesh mesh;
Mesh mesh2;

Texture albedo;
Texture emission;
Texture albedo2;
Texture emission2;
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
    
    ShaderDescriptions descriptionInfo = {};
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRGB32,offsetof(Vertex, color)},
        {SRG32, offsetof(Vertex, uv)}
    };
    descriptionInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = VK_COMPARE_OP_LESS_OR_EQUAL;
    
    Renderer::CreateShader({"EngineAssets/Shaders/triangleShader.frag", "EngineAssets/Shaders/triangleShader.vert"}, "triangle shader", {"triangle camera layout", "triangle object layout","triangle color layout"},&descriptionInfo);

    Renderer::CreateShader({"EngineAssets/Shaders/triangleShader.frag", "EngineAssets/Shaders/triangleShader.vert"}, "triangle shader2", {"triangle camera layout", "triangle object layout","triangle color layout"},&descriptionInfo);
    Renderer::CreateSampler("default sampler", FILTER_NEAREST);
    albedo.CreateTexture("EngineAssets/Textures/ExplosionBarrel Diffuse.png");
    emission.CreateTexture("EngineAssets/Textures/ExplosionBarrel Emission.png");

    Renderer::CreateShaderUniformBuffer("material buffer", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(TriangleData));
    Renderer::CreateShaderUniformBuffer("material buffer2", false, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(TriangleData));
    Renderer::CreateShaderUniformBuffer("camera buffer", true, BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GPUCameraData));
    const int MAX_OBJECTS = 10000;
    Renderer::CreateShaderUniformBuffer("mesh buffer", true, BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPUObjectData) * MAX_OBJECTS);

    Renderer::WriteShaderUniform("triangle color", "triangle color layout",0,false,"material buffer");
    Renderer::WriteShaderImage("triangle color", "triangle color layout", 1, "default sampler", {albedo.imageView, emission.imageView});
    albedo2.CreateTexture("EngineAssets/Textures/viking_room.png");
    emission2.CreateTexture("");
    Renderer::WriteShaderUniform("triangle color2", "triangle color layout",0,false,"material buffer2");
    Renderer::WriteShaderImage("triangle color2", "triangle color layout", 1, "default sampler", {albedo2.imageView, emission2.imageView});
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
            Renderer::UploadUniformDataToShader("mesh buffer",objectData, true);

           
            triangleData.color = glm::vec4(1.0f);
            
            std::vector<TriangleData> triangleDataArray = {triangleData};
            
            Renderer::UploadUniformDataToShader("material buffer",triangleDataArray, false);
            Renderer::UploadUniformDataToShader("material buffer2",triangleDataArray, false);
            
            Renderer::BindShader("triangle shader");
            
            Renderer::BindUniforms("camera data", "triangle shader", 0, true);
            Renderer::BindUniforms("object data", "triangle shader", 1,true);
            Renderer::BindUniforms("triangle color", "triangle shader",2,false);
            Renderer::BindVertexBuffer(mesh.vertexBuffer);
            Renderer::BindIndexBuffer(mesh.indexBuffer);
            Renderer::DrawIndexed(mesh.indices);

            Renderer::BindUniforms("triangle color2", "triangle shader",2,false);
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
            Renderer::DestroySampler("default sampler");
            albedo.DestroyTexture();
            emission.DestroyTexture();
            albedo2.DestroyTexture();
            emission2.DestroyTexture();
            mesh.DestroyMesh();
            mesh2.DestroyMesh();
            Renderer::RemoveAllocatedBuffer("mesh buffer", true);
            Renderer::RemoveAllocatedBuffer("material buffer", false);
            Renderer::RemoveAllocatedBuffer("material buffer2", false);
            Renderer::RemoveAllocatedBuffer("camera buffer", true);
            
        });
        Renderer::CleanUpRenderer(&additionalDeletion);
		windowHandler.DestroyWindow();
    }
}