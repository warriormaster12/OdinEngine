#include "Include/geometry_pipeline.h"

#include "camera.h"
#include "glm/gtx/transform.hpp"
#include "logger.h"
#include "render_object.h"
#include "material_manager.h"
#include "renderer.h"

#include "light.h"
#include "texture.h"
#include <memory>

RenderObject skybox;
Texture skyboxCubemap;

void GeometryPipeline::Init()
{

    

    ShaderDescriptions descriptionInfo3;
    descriptionInfo3.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
    };
    descriptionInfo3.cullMode = CULL_MODE_FRONT_BIT;
    descriptionInfo3.depthTesting = true;
    descriptionInfo3.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;
    descriptionInfo3.renderPassName = "test pass";

    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("camera layout");

    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayout("Cube texture layout");

    Renderer::CreateShader({"EngineAssets/Shaders/skybox.frag", "EngineAssets/Shaders/skybox.vert"}, "cube map", {"camera layout", "Cube texture layout"},&descriptionInfo3);
    
    Renderer::CreateSampler("default sampler", FILTER_NEAREST, SAMPLER_ADDRESS_MODE_REPEAT);

    skyboxCubemap.CreateCubeMapTexture({
        "EngineAssets/Textures/skybox/right.jpg",
        "EngineAssets/Textures/skybox/left.jpg",
        "EngineAssets/Textures/skybox/top.jpg",
        "EngineAssets/Textures/skybox/bottom.jpg",
        "EngineAssets/Textures/skybox/front.jpg",
        "EngineAssets/Textures/skybox/back.jpg"
    });


    Renderer::CreateSampler("cube map sampler", FILTER_NEAREST, SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    Renderer::WriteShaderUniform("camera data", "per frame layout",0,true,CameraManager::GetActiveCamera().GetCameraBuffer());

    Renderer::WriteShaderUniform("camera cube data", "camera layout",0,true,CameraManager::GetActiveCamera().GetCameraBuffer());

    Renderer::WriteShaderImage("cube map texture", "Cube texture layout", 0, "cube map sampler", {skyboxCubemap.imageView});

    skybox.p_mesh->LoadFromObj("EngineAssets/Meshes/cube.obj");
    skybox.p_mesh->CreateMesh();
    skybox.material = "cube";
    skybox.transformMatrix = glm::translate(glm::vec3(0));

    ObjectManager::PushObjectToQueue(skybox, "skyBox");

    ENGINE_CORE_INFO("geometry pipeline created");
}

void GeometryPipeline::Update()
{
    Renderer::PrepareRenderpassForDraw(2, {0.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
    Renderer::AddDrawToRenderpassQueue([=] {ObjectManager::RenderObjects();});
}

void GeometryPipeline::Destroy()
{
   
    skyboxCubemap.DestroyTexture();
    LightManager::Destroy();
    ObjectManager::Destroy();
    Renderer::DestroySampler("default sampler");
    Renderer::DestroySampler("cube map sampler");
    ENGINE_CORE_INFO("geometry pipeline destroyed");
}