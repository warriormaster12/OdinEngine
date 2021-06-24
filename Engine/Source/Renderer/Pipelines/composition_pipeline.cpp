#include "Include/composition_pipeline.h"

#include "logger.h"
#include "render_object.h"
#include "material_manager.h"
#include "renderer.h"

#include <memory>


void CompositionPipeline::Init()
{
    std::unique_ptr<FrameBufferInfo> bufferInfo = std::make_unique<FrameBufferInfo>();
    bufferInfo->width = 1280;
    bufferInfo->height = 720;
    bufferInfo->resiziable = false;
    bufferInfo->renderPassName = "test pass";
    bufferInfo->imageCount = 2;
    bufferInfo->imageLayouts = {IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    bufferInfo->colorFormats = {UNORMRGB8, {}};
    Renderer::CreateFramebuffer(FRAMEBUFFER_OFFSCREEN, "test offscreen", std::move(bufferInfo));

    

    ENGINE_CORE_INFO("composition pipeline created");

}

void CompositionPipeline::Update()
{
    Renderer::PrepareRenderpassForDraw(2, {0.0f, 0.0f, 0.0f,1.0f}, 1.0f, "test pass", "test offscreen");
    Renderer::AddDrawToRenderpassQueue([=]{
        ObjectManager::RenderObjects();
    }, "test pass");
}

void CompositionPipeline::Destroy()
{

}