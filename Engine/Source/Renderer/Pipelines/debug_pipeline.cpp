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

    Renderer::CreateShaderUniformLayoutBinding(UniformType::UNIFORM_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 0);
    Renderer::CreateShaderUniformLayout("output image layout");

    Renderer::CreateShader({"EngineAssets/Shaders/Contrast.frag", "EngineAssets/Shaders/debugQuad.vert"}, "debug shader", {"output image layout"}, &debugShaderDescription);

    Renderer::CreateSampler("debug sampler", FILTER_LINEAR,  SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    Renderer::WriteShaderFrameBufferImage("debug image", "output image layout", 0, "debug sampler", "test offscreen");

    ENGINE_CORE_INFO("debug pipeline created");

}

void DebugPipeline::Update()
{
    if(windowHandler.GetKInput(GLFW_KEY_1) == GLFW_PRESS)
    {
        debug = true;
    }
    else {
        debug = false;
    }
    if(debug == true)
    {
        
    }
    Renderer::PrepareRenderpassForDraw(2, {0.0f, 0.0f, 0.0f,1.0f}, 1.0f);
    Renderer::AddDrawToRenderpassQueue([=]{
        Renderer::BindShader("debug shader");
        Renderer::BindUniforms("debug image", 0);
        Renderer::Draw(3,1,0,0);
    });
}

void DebugPipeline::Destroy()
{
    Renderer::DestroySampler("debug sampler");
    //Renderer::RemoveShaderUniformLayout("output image");
}