#include "Include/vk_offscreen.h"
#include "Include/vk_renderer.h"
#include "vk_pipelinebuilder.h"
#include "vk_utils.h"

vkcomponent::PipelineBuilder offscreenBuilder;
FrameData frames[FRAME_OVERLAP];
FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

namespace 
{
	void UploadLightData(const VmaAllocator& allocator, const VmaAllocation& allocation)
    {
        LightMatrixData lightData;
		glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 96.0f);
		glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(100.0f, 100.0f, 100.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));
        lightData.lightSpaceMatrix = depthProjectionMatrix * depthViewMatrix;
		UploadSingleData(allocator, allocation, lightData);
    }
	void UploadDrawCalls(const VmaAllocator& allocator, const VmaAllocation& allocation, const std::vector<RenderObject>& objects)
    {
        std::vector<VkDrawIndirectCommand> commands;
        commands.reserve(objects.size());
        for (size_t i = 0; i < objects.size(); ++i)
        {
            VkDrawIndirectCommand cmd;
            cmd.vertexCount = objects[i].p_mesh->vertices.size();
            cmd.instanceCount = 1;
            cmd.firstVertex = 0;
            cmd.firstInstance = i;
            commands.push_back(cmd);
        }

		UploadVectorData(allocator, allocation, commands);
    }
	std::vector<DrawCall> BatchDrawCalls(const std::vector<RenderObject>& objects, const DescriptorSetData& descriptorSets)
    {
        std::vector<DrawCall> batch;

		Mesh* pLastMesh = nullptr;
		Material* pLastMaterial = nullptr;

        for (size_t i = 0; i < objects.size(); ++i)
        {
			bool isSameMesh = objects[i].p_mesh == pLastMesh;
			bool isSameMaterial = objects[i].p_material == pLastMaterial;

			if (i == 0 || !isSameMesh || !isSameMaterial) {
                DrawCall dc;
                dc.pMesh = objects[i].p_mesh;
                dc.pMaterial = objects[i].p_material;
                dc.descriptorSets = descriptorSets;
                dc.transformMatrix = objects[0].transformMatrix;
                dc.index = i;
                dc.count = 1;
                batch.push_back(dc);

				pLastMesh = objects[i].p_mesh;
				pLastMaterial = objects[i].p_material;
			} else {
				++batch.back().count;
			}
        }

        return batch;
    }

	void IssueDrawCalls(const VkCommandBuffer& cmd, const VkBuffer& drawCommandBuffer, const std::vector<DrawCall>& drawCalls)
	{
		for (const DrawCall& dc : drawCalls)
		{
			uint32_t stride = sizeof(VkDrawIndirectCommand);
			uint32_t offset = dc.index * stride;

			vkCmdDrawIndirect(cmd, drawCommandBuffer, offset, dc.count, stride);
		}
	}
}

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
	depth_attachment.format = VK_FORMAT_D16_UNORM;
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

	VK_CHECK(vkCreateRenderPass(p_renderer->GetDevice(), &render_pass_info, nullptr, &shadow.shadowPass));

	p_renderer->EnqueueCleanup([=]() {
		vkDestroyRenderPass(p_renderer->GetDevice(), shadow.shadowPass, nullptr);
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
		VkExtent3D shadowExtent3D = {shadow.shadowExtent.width, shadow.shadowExtent.height, 1};
	
		//the depth image will be a image with the format we selected and Depth Attachment usage flag
		VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, shadowExtent3D);

		//allocate and create the image
		vmaCreateImage(p_renderer->GetAllocator(), &dimg_info, &dimg_allocinfo, &shadow.shadowImage.image.image, &shadow.shadowImage.image.allocation, nullptr);

		//build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo dview_info = vkinit::ImageViewCreateInfo(VK_FORMAT_D16_UNORM, shadow.shadowImage.image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

		VK_CHECK(vkCreateImageView(p_renderer->GetDevice(), &dview_info, nullptr, &shadow.shadowImage.imageView));
	}
    VkFramebufferCreateInfo sh_info = vkinit::FramebufferCreateInfo(shadow.shadowPass, shadow.shadowExtent);
	sh_info.pAttachments = &shadow.shadowImage.imageView;
	sh_info.attachmentCount = 1;
	VK_CHECK(vkCreateFramebuffer(p_renderer->GetDevice(), &sh_info, nullptr, &shadow.shadowFramebuffer));
	VkFilter shadowFilter = vkinit::FormatIsFilterable(p_renderer->GetPhysicalDevice(), VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL) ?
		   VK_FILTER_LINEAR :
		   VK_FILTER_NEAREST;
	VkSamplerCreateInfo shadowInfo = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	vkCreateSampler(p_renderer->GetDevice(), &shadowInfo, nullptr, &shadow.shadowMapSampler);

    p_renderer->EnqueueCleanup([=]() {
		vkDestroyImageView(p_renderer->GetDevice(), shadow.shadowImage.imageView, nullptr);
		vkDestroyImage(p_renderer->GetDevice(), shadow.shadowImage.image.image, nullptr);
		vkDestroyFramebuffer(p_renderer->GetDevice(), shadow.shadowFramebuffer, nullptr);
		vkDestroySampler(p_renderer->GetDevice(), shadow.shadowMapSampler, nullptr);
	});

}

void VulkanOffscreen::InitDescriptors()
{
	VkDescriptorSetLayoutBinding debugTexBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	std::vector<VkDescriptorSetLayoutBinding> bindings = {debugTexBind};
	VkDescriptorSetLayoutCreateInfo _set1 = vkinit::DescriptorLayoutInfo(bindings);
	debugTextureLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set1);

	VkDescriptorSetLayoutBinding lightBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
	VkDescriptorSetLayoutBinding objectBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	std::vector<VkDescriptorSetLayoutBinding> lightBindings = {lightBind};
	VkDescriptorSetLayoutCreateInfo _set2 = vkinit::DescriptorLayoutInfo(lightBindings);
	shadow.offscreenGlobalSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set2);
	std::vector<VkDescriptorSetLayoutBinding> objectBindings = {objectBind};
	VkDescriptorSetLayoutCreateInfo _set3 = vkinit::DescriptorLayoutInfo(objectBindings);
	shadow.offscreenObjectSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set3);

	frames[0] = p_renderer->GetFrameData(0);
	frames[1] = p_renderer->GetFrameData(1);

	const int MAX_OBJECTS = 10000;
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		
		frames[i].p_dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		frames[i].p_dynamicDescriptorAllocator->Init(p_renderer->GetDevice());
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].globalDescriptor, shadow.offscreenGlobalSetLayout);
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].objectDescriptor, shadow.offscreenObjectSetLayout);

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

		VkDescriptorBufferInfo lightInfo;
		lightInfo.buffer = frames[i].cameraBuffer.buffer;
		lightInfo.offset = 0;
		lightInfo.range = sizeof(LightMatrixData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;
		
		vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), frames[i].p_dynamicDescriptorAllocator)
		.BindBuffer(0, &lightInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
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
	p_renderer->GetDescriptorAllocator()->Allocate(&debugTextureSet, debugTextureLayout);
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = shadow.shadowMapSampler;
	imageBufferInfo.imageView = shadow.shadowImage.imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), p_renderer->GetDescriptorAllocator())
	.BindImage(1, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	.Build(debugTextureSet);
}

void VulkanOffscreen::InitPipelines()
{
    //VkShaderModule debugVertexShader;
	vkcomponent::ShaderModule debugVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/debug.vert").c_str(), &debugVertShader, p_renderer->GetDevice());

	//VkShaderModule debugFragmentShader;
	vkcomponent::ShaderModule debugFragShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/debug.frag").c_str(), &debugFragShader, p_renderer->GetDevice());

	vkcomponent::ShaderEffect* shadowDebugEffect = new vkcomponent::ShaderEffect();
	std::vector<vkcomponent::ShaderModule> shaderModules = {debugVertShader, debugFragShader};

    
	std::array<VkDescriptorSetLayout, 1> layouts= {debugTextureLayout};
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


	//VkShaderModule offscreenVertex;
	vkcomponent::ShaderModule offscreenVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/offscreen.vert").c_str(), & offscreenVertShader, p_renderer->GetDevice());

	vkcomponent::ShaderEffect* offscreenEffect = new vkcomponent::ShaderEffect();
	shaderModules.clear();
	shaderModules = {offscreenVertShader};

    
	std::array<VkDescriptorSetLayout, 2> offscreenLayouts= {shadow.offscreenGlobalSetLayout, shadow.offscreenObjectSetLayout};
	VkPipelineLayoutCreateInfo offscreenpipInfo = vkinit::PipelineLayoutCreateInfo();
	offscreenpipInfo.pSetLayouts = offscreenLayouts.data();
	offscreenpipInfo.setLayoutCount = offscreenLayouts.size();
	offscreenEffect = vkcomponent::BuildEffect(p_renderer->GetDevice(), shaderModules, offscreenpipInfo);

	std::vector <LocationInfo> locations = {{VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position)}
	};
	VertexInputDescription vertexDescription = Vertex::GetVertexDescription(locations);

	//connect the pipeline builder vertex input info to the one we get from Vertex
	offscreenBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	offscreenBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	offscreenBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	offscreenBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	offscreenBuilder.rasterizer.depthBiasEnable = true;


	vkcomponent::ShaderPass* offscreenPass = vkcomponent::BuildShader(p_renderer->GetDevice(), shadow.shadowPass, offscreenBuilder, offscreenEffect);
	shadow.shadowPipeline = offscreenPass->pipeline;
	shadow.shadowPipelineLayout = offscreenPass->layout;


    p_renderer->EnqueueCleanup([=]()
    {
        debugPass->FlushPass(p_renderer->GetDevice());
		offscreenPass->FlushPass(p_renderer->GetDevice());
    });
	
}

void VulkanOffscreen::BeginOffscreenRenderpass()
{
	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;	
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(shadow.shadowPass, shadow.shadowExtent, shadow.shadowFramebuffer);

	//connect clear values
	rpInfo.clearValueCount = 1;

	VkClearValue clearValues[] = { depthClear };

	rpInfo.pClearValues = &clearValues[0];
	vkCmdBeginRenderPass(p_renderer->GetCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanOffscreen::debugShadows(bool /*debug = false*/)
{
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)p_renderer->GetWidth();
	viewport.height = (float)p_renderer->GetHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkExtent2D imageExtent;
	imageExtent.width = p_renderer->GetWidth();
	imageExtent.height = p_renderer->GetHeight();
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = imageExtent;

	vkCmdSetViewport(p_renderer->GetCommandBuffer(), 0, 1, &viewport);
	vkCmdSetScissor(p_renderer->GetCommandBuffer(), 0, 1, &scissor);
	vkCmdBindPipeline(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebug);
	
	vkCmdBindDescriptorSets(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebugLayout, 0,1, &debugTextureSet, 0, nullptr);
	
	vkCmdDraw(p_renderer->GetCommandBuffer(), 3, 1, 0, 0);

}

void VulkanOffscreen::EndOffscreenRenderpass()
{
	vkCmdEndRenderPass(p_renderer->GetCommandBuffer());
}