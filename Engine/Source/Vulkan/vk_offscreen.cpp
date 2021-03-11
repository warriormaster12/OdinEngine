#include "Include/vk_offscreen.h"
#include "Include/vk_renderer.h"
#include "vk_pipelinebuilder.h"

vkcomponent::PipelineBuilder offscreenBuilder;

void VulkanOffscreen::InitOffscreen(VulkanRenderer& renderer)
{
   p_renderer = &renderer;

   InitRenderpass();
   InitFramebuffer();
   InitDescriptors();
   InitPipelines();
}

void VulkanOffscreen::InitRenderpass()
{
    //shadow pass
	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = p_renderer->GetSwapChain().depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment =0;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//1 dependency, which is from "outside" into the subpass. And we can read or write color
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &depth_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;	

	VK_CHECK(vkCreateRenderPass(p_renderer->GetDevice(), &render_pass_info, nullptr, &shadowPass));

	p_renderer->EnqueueCleanup([=]() {
		vkDestroyRenderPass(p_renderer->GetDevice(), shadowPass, nullptr);
	});
}


void VulkanOffscreen::InitFramebuffer()
{
    //We for now create shadow image before framebuffer. Normally this would be done when creating the swapchain. 

	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	//shadow image
	{
		VkExtent3D shadowExtent3D = {shadowExtent.width, shadowExtent.height, 1};
	
		//the depth image will be a image with the format we selected and Depth Attachment usage flag
		VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(p_renderer->GetSwapChain().depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, shadowExtent3D);

		//allocate and create the image
		vmaCreateImage(p_renderer->GetAllocator(), &dimg_info, &dimg_allocinfo, &shadowImage.image.image, &shadowImage.image.allocation, nullptr);

		//build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo dview_info = vkinit::ImageViewCreateInfo(p_renderer->GetSwapChain().depthFormat, shadowImage.image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

		VK_CHECK(vkCreateImageView(p_renderer->GetDevice(), &dview_info, nullptr, &shadowImage.imageView));
	}

    VkFramebufferCreateInfo sh_info = vkinit::FramebufferCreateInfo(shadowPass, shadowExtent);
	sh_info.pAttachments = &shadowImage.imageView;
	sh_info.attachmentCount = 1;
	VK_CHECK(vkCreateFramebuffer(p_renderer->GetDevice(), &sh_info, nullptr, &shadowFramebuffer));

    p_renderer->EnqueueCleanup([=]() {
		vkDestroyImageView(p_renderer->GetDevice(), shadowImage.imageView, nullptr);
		vkDestroyImage(p_renderer->GetDevice(), shadowImage.image.image, nullptr);
		vkDestroyFramebuffer(p_renderer->GetDevice(), shadowFramebuffer, nullptr);
	}, &p_renderer->GetSwapQueue());

}

void VulkanOffscreen::InitDescriptors()
{

}

void VulkanOffscreen::InitPipelines()
{
    //VkShaderModule texturedMeshShader;
	vkcomponent::ShaderModule texturedMeshShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_pbr_lit.frag").c_str(), &texturedMeshShader, p_renderer->GetDevice());

	//VkShaderModule meshVertShader;
	vkcomponent::ShaderModule meshVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_triangle.vert").c_str(), &meshVertShader, p_renderer->GetDevice());

	vkcomponent::ShaderEffect* defaultEffect = new vkcomponent::ShaderEffect();
	std::vector<vkcomponent::ShaderModule> shaderModules = {meshVertShader, texturedMeshShader};

    //for now pipeline layout is going to be null because our shader doesn't communicate with descriptors
	//std::array<VkDescriptorSetLayout, 3> layouts= {globalSetLayout, objectSetLayout, materialTextureSetLayout};
	VkPipelineLayoutCreateInfo meshpipInfo = vkinit::PipelineLayoutCreateInfo();
	meshpipInfo.pSetLayouts = nullptr;
	meshpipInfo.setLayoutCount = 0;
	defaultEffect = vkcomponent::BuildEffect(p_renderer->GetDevice(), shaderModules, meshpipInfo);

	//hook the push constants layout
	offscreenBuilder.pipelineLayout = defaultEffect->builtLayout;
	//we have copied layout to builder so now we can flush old one
	defaultEffect->FlushLayout();

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	offscreenBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	offscreenBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	offscreenBuilder.viewport.x = 0.0f;
	offscreenBuilder.viewport.y = 0.0f;
	offscreenBuilder.viewport.width = (float)shadowExtent.width;
	offscreenBuilder.viewport.height = (float)shadowExtent.height;
	offscreenBuilder.viewport.minDepth = 0.0f;
	offscreenBuilder.viewport.maxDepth = 1.0f;

	offscreenBuilder.blendConstant = {0.0f, 0.0f, 0.0f, 0.0f};

	offscreenBuilder.scissor.offset = { 0, 0 };
	offscreenBuilder.scissor.extent = shadowExtent;

	//configure the rasterizer to draw filled triangles
	offscreenBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	offscreenBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	offscreenBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();


	//default depthtesting
	offscreenBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the mesh pipeline

	//VertexInputDescription vertexDescription = Vertex::GetVertexDescription(1);

	//connect the pipeline builder vertex input info to the one we get from Vertex
	// pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	// pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	// pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	// pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	
	//build the mesh triangle pipeline
	vkcomponent::ShaderPass* defaultPass = vkcomponent::BuildShader(p_renderer->GetDevice(), shadowPass, offscreenBuilder, defaultEffect);
    offscreenBuilder.BuildPipeline(p_renderer->GetDevice(), shadowPass);

    p_renderer->EnqueueCleanup([=]()
    {
        defaultPass->FlushPass(p_renderer->GetDevice());
    });
}

void VulkanOffscreen::BeginOffscreenDraw()
{

}

void VulkanOffscreen::EndOffscreenDraw()
{

}