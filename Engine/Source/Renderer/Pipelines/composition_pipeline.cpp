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

    ShaderDescriptions descriptionInfo;
    descriptionInfo.vertexLocations = {
        {SRGB32,offsetof(Vertex, position)},
        {SRG32, offsetof(Vertex, uv)},
        {SRGB32,offsetof(Vertex, normal)}
    };
    descriptionInfo.cullMode = CULL_MODE_BACK_BIT;
    descriptionInfo.depthTesting = true;
    descriptionInfo.depthCompareType = COMPARE_OP_LESS;
    descriptionInfo.renderPassName = "test pass";

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
    descriptionInfo2.depthCompareType = COMPARE_OP_LESS;
    descriptionInfo2.renderPassName = "test pass";

    Renderer::CreateShader({"EngineAssets/Shaders/defaultWorld.frag", "EngineAssets/Shaders/pbr_vert.vert"}, "default world", {"per frame layout", "per object layout"},&descriptionInfo2);

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