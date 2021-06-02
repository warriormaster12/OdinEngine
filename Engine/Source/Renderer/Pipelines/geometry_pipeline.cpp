#include "Include/geometry_pipeline.h"

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

    ObjectManager::Init();
    MaterialManager::Init();
    LightManager::Init();

    ShaderDescriptions descriptionInfo;
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRG32, offsetof(Vertex, uv)},
        {SRGB32,offsetof(Vertex, normal)}
    };
    descriptionInfo.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;

    descriptionInfo.p_pushConstant = std::make_unique<PushConstant>();
    descriptionInfo.p_pushConstant->dataSize = sizeof(TextureCheck);
    descriptionInfo.p_pushConstant->offset = 0;
    descriptionInfo.p_pushConstant->shaderStage = SHADER_STAGE_FRAGMENT_BIT;
    
    Renderer::CreateShader({"EngineAssets/Shaders/defaultTexturedWorld.frag", "EngineAssets/Shaders/pbr_vert.vert"}, "default textured world", {"per frame layout", "per object layout", "texture data layout"},&descriptionInfo);

    ShaderDescriptions descriptionInfo2;
    descriptionInfo2.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRG32, offsetof(Vertex, uv)},
        {SRGB32,offsetof(Vertex, normal)}
    };
    descriptionInfo2.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo2.depthTesting = true;
    descriptionInfo2.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;

    Renderer::CreateShader({"EngineAssets/Shaders/defaultWorld.frag", "EngineAssets/Shaders/pbr_vert.vert"}, "default world", {"per frame layout", "per object layout"},&descriptionInfo2);

    ShaderDescriptions descriptionInfo3;
    descriptionInfo3.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
    };
    descriptionInfo3.cullMode = CULL_MODE_FRONT_BIT;
    descriptionInfo3.depthTesting = true;
    descriptionInfo3.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;

    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 0);
    Renderer::CreateShaderUniformLayout("camera layout");

    Renderer::CreateShaderUniformLayoutBinding(UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 0);
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

    Renderer::WriteShaderUniform("cube camera", "camera layout",0,true,"camera buffer cube");

    Renderer::WriteShaderImage("cube map texture", "Cube texture layout", 0, "cube map sampler", {skyboxCubemap.imageView});

    skybox.p_mesh = new Mesh;
    skybox.p_mesh->LoadFromObj("EngineAssets/Meshes/cube.obj");
    skybox.p_mesh->CreateMesh();
    skybox.material = "cube";
    skybox.transformMatrix = glm::translate(glm::vec3(0));

    ObjectManager::PushObjectToQueue(skybox);

    ENGINE_CORE_INFO("geometry pipeline created");
}

void GeometryPipeline::Update()
{
    ObjectManager::RenderObjects();
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