#include "Include/geometryPipeline.h"

#include "logger.h"
#include "renderobject.h"
#include "renderer.h"

void GeometryPipeline::Init()
{

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

    ENGINE_CORE_INFO("geometry pipeline created");
}

void GeometryPipeline::Update()
{
    
}

void GeometryPipeline::Destroy()
{
    Renderer::DestroySampler("default sampler");
    ENGINE_CORE_INFO("geometry pipeline destroyed");
}