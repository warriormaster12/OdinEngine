#include "Include/vk_offscreen.h"
#include "Include/vk_renderer.h"
#include "vk_pipelinebuilder.h"
#include "vk_utils.h"

vkcomponent::PipelineBuilder offscreenBuilder;
FrameData frames[FRAME_OVERLAP];

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

	VkSamplerCreateInfo shadowInfo = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	vkCreateSampler(p_renderer->GetDevice(), &shadowInfo, nullptr, &shadowMapSampler);

    p_renderer->EnqueueCleanup([=]() {
		vkDestroyImageView(p_renderer->GetDevice(), shadowImage.imageView, nullptr);
		vkDestroyImage(p_renderer->GetDevice(), shadowImage.image.image, nullptr);
		vkDestroyFramebuffer(p_renderer->GetDevice(), shadowFramebuffer, nullptr);
		vkDestroySampler(p_renderer->GetDevice(), shadowMapSampler, nullptr);
	}, &p_renderer->GetSwapQueue());

}

void VulkanOffscreen::InitDescriptors()
{
	VkDescriptorSetLayoutBinding cameraBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
	VkDescriptorSetLayoutBinding debugTexBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	std::vector<VkDescriptorSetLayoutBinding> bindings = { debugTexBind};
	VkDescriptorSetLayoutCreateInfo _set1 = vkinit::DescriptorLayoutInfo(bindings);
	offscreenGlobalSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set1);

	VkDescriptorSetLayoutBinding objectBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	std::vector<VkDescriptorSetLayoutBinding> objectBindings = { objectBind};
	VkDescriptorSetLayoutCreateInfo _set2 = vkinit::DescriptorLayoutInfo(objectBindings);
	offscreenObjectSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set2);

	frames[0] = p_renderer->GetFrameData(0);
	frames[1] = p_renderer->GetFrameData(1);

	const int MAX_OBJECTS = 10000;
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		
		frames[i].p_dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		frames[i].p_dynamicDescriptorAllocator->Init(p_renderer->GetDevice());
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].globalDescriptor, offscreenGlobalSetLayout);
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].objectDescriptor, offscreenObjectSetLayout);

		{
			CreateBufferInfo info;
			info.allocSize = sizeof(LightMatrixData);
			info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(p_renderer->GetAllocator(), &frames[i].cameraBuffer, info);
		}

		{
			CreateBufferInfo info;
			info.allocSize = MAX_OBJECTS * sizeof(GPUObjectData);
			info.bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            CreateBuffer(p_renderer->GetAllocator(), &frames[i].objectBuffer, info);
		}
		// {
		// 	CreateBufferInfo info;
		// 	info.allocSize = MAX_COMMANDS * sizeof(VkDrawIndirectCommand);
		// 	info.bufferUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		// 	info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		// 	CreateBuffer(allocator, &frames[i].indirectDrawBuffer, info);
		// }

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = frames[i].cameraBuffer.buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(LightMatrixData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;
		VkDescriptorImageInfo imageBufferInfo;
		imageBufferInfo.sampler = shadowMapSampler;
		imageBufferInfo.imageView = shadowImage.imageView;
		imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), frames[i].p_dynamicDescriptorAllocator)
		.BindImage(1, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build(frames[i].globalDescriptor);

		vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), frames[i].p_dynamicDescriptorAllocator)
		.BindBuffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(frames[i].objectDescriptor);

		p_renderer->EnqueueCleanup([=]()
		{
			vmaDestroyBuffer(p_renderer->GetAllocator(), frames[i].objectBuffer.buffer, frames[i].objectBuffer.allocation);
			vmaDestroyBuffer(p_renderer->GetAllocator(), frames[i].cameraBuffer.buffer, frames[i].cameraBuffer.allocation);

			frames[i].p_dynamicDescriptorAllocator->CleanUp();
		});
	}
}

void VulkanOffscreen::InitPipelines()
{
    //VkShaderModule texturedMeshShader;
	vkcomponent::ShaderModule debugVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/debug.vert").c_str(), &debugVertShader, p_renderer->GetDevice());

	//VkShaderModule meshVertShader;
	vkcomponent::ShaderModule debugFragShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/debug.frag").c_str(), &debugFragShader, p_renderer->GetDevice());

	vkcomponent::ShaderEffect* shadowDebugEffect = new vkcomponent::ShaderEffect();
	std::vector<vkcomponent::ShaderModule> shaderModules = {debugVertShader, debugFragShader};

    
	std::array<VkDescriptorSetLayout, 1> layouts= {offscreenGlobalSetLayout};
	VkPipelineLayoutCreateInfo debugpipInfo = vkinit::PipelineLayoutCreateInfo();
	debugpipInfo.pSetLayouts = layouts.data();
	debugpipInfo.setLayoutCount = layouts.size();
	shadowDebugEffect = vkcomponent::BuildEffect(p_renderer->GetDevice(), shaderModules, debugpipInfo);

	//hook the push constants layout
	offscreenBuilder.pipelineLayout = shadowDebugEffect->builtLayout;
	//we have copied layout to builder so now we can flush old one
	shadowDebugEffect->FlushLayout();

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	offscreenBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

	offscreenBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//configure the rasterizer to draw filled triangles
	offscreenBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE);

	offscreenBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

	offscreenBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the debug pipeline
	vkcomponent::ShaderPass* debugPass = vkcomponent::BuildShader(p_renderer->GetDevice(), p_renderer->GetRenderPass(), offscreenBuilder, shadowDebugEffect);
	shadowDebug = debugPass->pipeline;
	shadowDebugLayout = debugPass->layout;
    p_renderer->EnqueueCleanup([=]()
    {
        debugPass->FlushPass(p_renderer->GetDevice());
    });
	
}

void VulkanOffscreen::BeginOffscreenRenderpass()
{
	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;	
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(shadowPass, shadowExtent, shadowFramebuffer);

	//connect clear values
	rpInfo.clearValueCount = 1;

	VkClearValue clearValues[] = { depthClear };

	rpInfo.pClearValues = &clearValues[0];
	vkCmdBeginRenderPass(p_renderer->GetCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)shadowExtent.width;
	viewport.height = (float)shadowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = shadowExtent;

	vkCmdSetViewport(p_renderer->GetCommandBuffer(), 0, 1, &viewport);
	vkCmdSetScissor(p_renderer->GetCommandBuffer(), 0, 1, &scissor);

	vkCmdBindPipeline(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebug);
	for(auto& frame : frames)
	{
		vkCmdBindDescriptorSets(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebugLayout, 0, 1, &frame.globalDescriptor, 0, nullptr);
	}
}

void VulkanOffscreen::EndOffscreenRenderpass()
{
	vkCmdEndRenderPass(p_renderer->GetCommandBuffer());
}