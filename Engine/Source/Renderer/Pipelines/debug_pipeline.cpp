#include "Include/debug_pipeline.h"
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
    //Renderer::CreateFramebuffer(FRAMEBUFFER_OFFSCREEN, "test offscreen", std::move(bufferInfo));
}

void DebugPipeline::Update()
{

}

void DebugPipeline::Destroy()
{

}