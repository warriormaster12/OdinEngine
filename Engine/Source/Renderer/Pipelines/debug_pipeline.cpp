#include "Include/debug_pipeline.h"
#include "logger.h"
#include "renderer.h"

#include "window_handler.h"

#include <memory>

bool debug = false;

void DebugPipeline::Init()
{
    ShaderDescriptions debugShaderDescription = {};
    debugShaderDescription.cullMode = CULL_MODE_FRONT_BIT;
    debugShaderDescription.depthTesting = false;
    debugShaderDescription.vertexLocations = {};

    Renderer::CreateShader({"EngineAssets/Shaders/debugQuad.frag", "EngineAssets/Shaders/debugQuad.vert"}, "debug shader", {}, &debugShaderDescription);

    ENGINE_CORE_INFO("debug pipeline created");

}

void DebugPipeline::Update()
{
    if(windowHandler.GetKInput(GLFW_KEY_1) == GLFW_PRESS)
    {
        if(debug == false)
        {
            debug = true;
        }
        else {
            debug = false;
        }
    }
    if(debug == true)
    {
        Renderer::PrepareRenderpassForDraw(2, {0.0f, 0.0f, 0.0f,1.0f}, 1.0f);
        Renderer::AddDrawToRenderpassQueue([=]{
            Renderer::BindShader("debug shader");
            Renderer::Draw(3,1,0,0);
        });
    }
}

void DebugPipeline::Destroy()
{

}