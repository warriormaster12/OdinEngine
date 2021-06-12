#include "Include/debug_pipeline.h"
#include "logger.h"
#include "renderer.h"

#include <memory>

void DebugPipeline::Init()
{
    Renderer::CreateRenderPass(RENDERPASS_OFFSCREEN, "test pass");
    std::unique_ptr<FrameBufferInfo> bufferInfo = std::make_unique<FrameBufferInfo>();
    bufferInfo->width = 512;
    bufferInfo->height = 512;
    bufferInfo->resiziable = false;
    bufferInfo->renderPassName = "test pass";
    bufferInfo->imageCount = 2;
    Renderer::CreateFramebuffer(FRAMEBUFFER_OFFSCREEN, "test offscreen", std::move(bufferInfo));

    ShaderDescriptions debugShaderDescription = {};
    debugShaderDescription.cullMode = CULL_MODE_FRONT_BIT;
    debugShaderDescription.depthTesting = false;
    debugShaderDescription.vertexLocations = {};
    debugShaderDescription.renderPassName = "test pass";

    Renderer::CreateShader({"EngineAssets/Shaders/debugQuad.frag", "EngineAssets/Shaders/debugQuad.vert"}, "debug shader", {}, &debugShaderDescription);

}

void DebugPipeline::Update()
{
    Renderer::BeginRenderpass(2, {0.0f, 0.0f, 0.0f,1.0f}, 1.0f, "test pass", "test offscreen");
    Renderer::BindShader("debug shader");
    Renderer::Draw(3,1,0,0);
    Renderer::EndRenderpass();

}

void DebugPipeline::Destroy()
{

}