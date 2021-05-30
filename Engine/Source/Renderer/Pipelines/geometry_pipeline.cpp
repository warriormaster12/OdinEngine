#include "Include/geometry_pipeline.h"

#include "logger.h"
#include "render_object.h"
#include "material_manager.h"
#include "renderer.h"

#include "light.h"

void GeometryPipeline::Init()
{

    ObjectManager::Init();
    MaterialManager::Init();

    ShaderDescriptions descriptionInfo;
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRGB32,offsetof(Vertex, color)},
        {SRG32, offsetof(Vertex, uv)}
    };
    descriptionInfo.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = COMPARE_OP_LESS_OR_EQUAL;
    
    Renderer::CreateShader({"EngineAssets/Shaders/defaultTexturedWorld.frag", "EngineAssets/Shaders/defaultTexturedWorld.vert"}, "default textured world", {"per frame layout", "per object layout", "texture data layout"},&descriptionInfo);

    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRG32, offsetof(Vertex, uv)},
        {SRGB32,offsetof(Vertex, normal)}
    };

    Renderer::CreateShader({"EngineAssets/Shaders/defaultWorld.frag", "EngineAssets/Shaders/defaultWorld.vert"}, "default world", {"per frame layout", "per object layout"},&descriptionInfo);
    Renderer::CreateSampler("default sampler", FILTER_NEAREST, SAMPLER_ADDRESS_MODE_REPEAT);

    LightManager::Init();

    ENGINE_CORE_INFO("geometry pipeline created");
}

void GeometryPipeline::Update()
{
    ObjectManager::RenderObjects();
}

void GeometryPipeline::Destroy()
{
    LightManager::Destroy();
    ObjectManager::Destroy();
    Renderer::DestroySampler("default sampler");
    ENGINE_CORE_INFO("geometry pipeline destroyed");
}