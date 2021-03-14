#include "Include/vk_offscreen.h"
#include "Include/vk_renderer.h"
#include "vk_pipelinebuilder.h"
#include "vk_utils.h"
#include <glm/gtx/matrix_decompose.hpp>

vkcomponent::PipelineBuilder offscreenBuilder;
FrameData frames[FRAME_OVERLAP];
FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

VkFormat depthFormat;

glm::vec3 lightPos = glm::vec3(0.0f, 50.0f, 50.0f);

namespace 
{
	void UploadLightData(const VmaAllocator& allocator, const VmaAllocation& allocation, std::array<Cascade, SHADOW_MAP_CASCADE_COUNT>& cascades)
    {
        Shadow::LightMatrixData lightData;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			lightData.cascadeViewProjMat[i] = cascades[i].viewProjMatrix;
		}
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
	void UploadObjectData(const VmaAllocator& allocator, const VmaAllocation& allocation, const std::vector<RenderObject>& objects)
    {
        std::vector<GPUObjectData> data;
        data.reserve(objects.size());
        for (const RenderObject& obj : objects)
        {
            GPUObjectData objData;
            objData.modelMatrix = obj.transformMatrix;
            data.push_back(objData);
        }

		UploadVectorData(allocator, allocation, data);
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

	void IssueDrawCalls(const VkCommandBuffer& cmd, const VkBuffer& drawCommandBuffer, const std::vector<DrawCall>& drawCalls, const VkPipeline& pipeline, const VkPipelineLayout& layout)
	{
		PushConstBlock pushConstBlock = {};
		pushConstBlock.cascadeIndex = SHADOW_MAP_CASCADE_COUNT - 1;

		for (const DrawCall& dc : drawCalls)
		{
			glm::vec3 position;
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 scew;
			glm::vec4 perspective;
			glm::decompose(dc.transformMatrix, scale, rotation, position, scew, perspective);
			pushConstBlock.position = glm::vec4(position, 0.0f);
			vkCmdPushConstants(cmd,layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &dc.descriptorSets.uniform, 0, nullptr);

			VkDeviceSize vertexOffset = 0;
        	vkCmdBindVertexBuffers(cmd, 0, 1, &dc.pMesh->vertexBuffer.buffer, &vertexOffset);

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
   BuildImage();

}

void VulkanOffscreen::InitRenderpass()
{
    depthFormat = vkinit::GetSupportedDepthFormat(true, p_renderer->GetPhysicalDevice());

	/*
		Depth map renderpass
	*/

	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = depthFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 0;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassCreateInfo.pDependencies = dependencies.data();

	VK_CHECK(vkCreateRenderPass(p_renderer->GetDevice(), &renderPassCreateInfo, nullptr, &shadow.shadowPass));


	p_renderer->EnqueueCleanup([=]() {
		vkDestroyRenderPass(p_renderer->GetDevice(), shadow.shadowPass, nullptr);
	});
}


void VulkanOffscreen::InitFramebuffer()
{
    /*
		Layered depth image and views
	*/
	VkExtent3D shadowExtent3D = {shadow.shadowExtent.width, shadow.shadowExtent.height, 1};
	VkImageCreateInfo imageInfo = vkinit::ImageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,shadowExtent3D);
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = SHADOW_MAP_CASCADE_COUNT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VK_CHECK(vmaCreateImage(p_renderer->GetAllocator(),&imageInfo, &dimg_allocinfo, &shadow.shadowImage.image.image, &shadow.shadowImage.image.allocation, nullptr));
	// Full depth map view (all layers)
	VkImageViewCreateInfo viewInfo = vkinit::ImageViewCreateInfo(depthFormat, shadow.shadowImage.image.image, VK_IMAGE_ASPECT_DEPTH_BIT);
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewInfo.subresourceRange = {};
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = SHADOW_MAP_CASCADE_COUNT;
	VK_CHECK(vkCreateImageView(p_renderer->GetDevice(), &viewInfo, nullptr, &shadow.shadowImage.imageView));

	// One image and framebuffer per cascade
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		// Image view for this cascade's layer (inside the depth map)
		// This view is used to render to that specific depth image layer
		VkImageViewCreateInfo viewInfo = vkinit::ImageViewCreateInfo(depthFormat, shadow.shadowImage.image.image, VK_IMAGE_ASPECT_DEPTH_BIT);
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		viewInfo.subresourceRange = {};
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = i;
		viewInfo.subresourceRange.layerCount = 1;
		VK_CHECK(vkCreateImageView(p_renderer->GetDevice(), &viewInfo, nullptr, &cascades[i].view));
		// Framebuffer
		VkFramebufferCreateInfo framebufferInfo = vkinit::FramebufferCreateInfo(shadow.shadowPass, shadow.shadowExtent);
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &cascades[i].view;
		framebufferInfo.layers = 1;
		VK_CHECK(vkCreateFramebuffer(p_renderer->GetDevice(), &framebufferInfo, nullptr, &cascades[i].frameBuffer));
		p_renderer->EnqueueCleanup([=]() {
			cascades[i].destroy(p_renderer->GetDevice());
		});
	}

    p_renderer->EnqueueCleanup([=]() {
		vkDestroyImageView(p_renderer->GetDevice(), shadow.shadowImage.imageView, nullptr);
		vkDestroyImage(p_renderer->GetDevice(), shadow.shadowImage.image.image, nullptr);
		vkDestroySampler(p_renderer->GetDevice(), shadow.shadowMapSampler, nullptr);
	});

}

void VulkanOffscreen::InitDescriptors()
{
	VkDescriptorSetLayoutBinding debugTexBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	
	std::vector<VkDescriptorSetLayoutBinding> bindings = {debugTexBind};
	VkDescriptorSetLayoutCreateInfo _set1 = vkinit::DescriptorLayoutInfo(bindings);
	debugSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set1);

	VkDescriptorSetLayoutBinding depthBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0);
	VkDescriptorSetLayoutBinding depthImageBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,1);

	std::vector<VkDescriptorSetLayoutBinding> lightBindings = {depthBind};
	VkDescriptorSetLayoutCreateInfo _set2 = vkinit::DescriptorLayoutInfo(lightBindings);
	depthSetLayout = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set2);

	std::vector<VkDescriptorSetLayoutBinding> depthImageBindings = {depthImageBind};
	VkDescriptorSetLayoutCreateInfo _set3 = vkinit::DescriptorLayoutInfo(depthImageBindings);
	depthSetLayoutGlobal = p_renderer->GetDescriptorLayoutCache()->CreateDescriptorLayout(&_set3);

	frames[0] = p_renderer->GetFrameData(0);
	frames[1] = p_renderer->GetFrameData(1);

	const int MAX_OBJECTS = 10000;
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		
		frames[i].p_dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		frames[i].p_dynamicDescriptorAllocator->Init(p_renderer->GetDevice());
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].globalDescriptor, depthSetLayoutGlobal);

		{
			CreateBufferInfo info;
			info.allocSize = sizeof(Shadow::LightMatrixData);
			info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(p_renderer->GetAllocator(), &frames[i].cameraBuffer, info);
		}

		VkDescriptorBufferInfo lightInfo;
		lightInfo.buffer = frames[i].cameraBuffer.buffer;
		lightInfo.offset = 0;
		lightInfo.range = sizeof(Shadow::LightMatrixData);

		
		vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), frames[i].p_dynamicDescriptorAllocator)
		.BindBuffer(0, &lightInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(frames[i].globalDescriptor);

		p_renderer->EnqueueCleanup([=]()
		{
			vmaDestroyBuffer(p_renderer->GetAllocator(), frames[i].cameraBuffer.buffer, frames[i].cameraBuffer.allocation);

			frames[i].p_dynamicDescriptorAllocator->CleanUp();
		});
	}
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

    
	std::array<VkDescriptorSetLayout, 1> layouts= {debugSetLayout};
	VkPipelineLayoutCreateInfo debugpipInfo = vkinit::PipelineLayoutCreateInfo();
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(PushConstBlock);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	debugpipInfo.pSetLayouts = layouts.data();
	debugpipInfo.setLayoutCount = layouts.size();
	debugpipInfo.pushConstantRangeCount = 1;
	debugpipInfo.pPushConstantRanges = &pushConstant;
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
	
	offscreenBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();

	offscreenBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the debug pipeline
	vkcomponent::ShaderPass* debugPass = vkcomponent::BuildShader(p_renderer->GetDevice(), p_renderer->GetRenderPass(), offscreenBuilder, shadowDebugEffect);
	shadowDebug = debugPass->pipeline;
	shadowDebugLayout = debugPass->layout;


	//VkShaderModule depthVertex;
	vkcomponent::ShaderModule depthVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/depthpass.vert").c_str(), & depthVertShader, p_renderer->GetDevice());
	//VkShaderModule depthFrag;
	vkcomponent::ShaderModule depthFragShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/depthpass.frag").c_str(), & depthFragShader, p_renderer->GetDevice());

	vkcomponent::ShaderEffect* offscreenEffect = new vkcomponent::ShaderEffect();
	shaderModules.clear();
	shaderModules = {depthVertShader, depthFragShader};

    
	std::array<VkDescriptorSetLayout, 2> offscreenLayouts= {depthSetLayout, depthSetLayoutGlobal};
	VkPipelineLayoutCreateInfo offscreenpipInfo = vkinit::PipelineLayoutCreateInfo();
	offscreenpipInfo.pSetLayouts = offscreenLayouts.data();
	offscreenpipInfo.setLayoutCount = offscreenLayouts.size();
	offscreenpipInfo.pushConstantRangeCount = 1;
	offscreenpipInfo.pPushConstantRanges = &pushConstant;
	offscreenEffect = vkcomponent::BuildEffect(p_renderer->GetDevice(), shaderModules, offscreenpipInfo);

	std::vector <LocationInfo> locations = {{VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position)},
		{VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, uv)}
	};
	VertexInputDescription vertexDescription = Vertex::GetVertexDescription(locations);


	//connect the pipeline builder vertex input info to the one we get from Vertex
	offscreenBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	offscreenBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	offscreenBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	offscreenBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	offscreenBuilder.rasterizer.depthClampEnable = true;


	vkcomponent::ShaderPass* offscreenPass = vkcomponent::BuildShader(p_renderer->GetDevice(), shadow.shadowPass, offscreenBuilder, offscreenEffect);
	shadow.shadowPipeline = offscreenPass->pipeline;
	shadow.shadowPipelineLayout = offscreenPass->layout;


    p_renderer->EnqueueCleanup([=]()
    {
        debugPass->FlushPass(p_renderer->GetDevice());
		offscreenPass->FlushPass(p_renderer->GetDevice());
    });
	
}

void VulkanOffscreen::BuildImage()
{
	// Shared sampler for cascade depth reads
	VkSamplerCreateInfo sampler = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK(vkCreateSampler(p_renderer->GetDevice(), &sampler, nullptr, &shadow.shadowMapSampler));

	p_renderer->GetDescriptorAllocator()->Allocate(&debugTextureSet, debugSetLayout);
	p_renderer->GetDescriptorAllocator()->Allocate(&depthTextureSet, depthSetLayout);
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = shadow.shadowMapSampler;
	imageBufferInfo.imageView = shadow.shadowImage.imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), p_renderer->GetDescriptorAllocator())
	.BindImage(0, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	.Build(debugTextureSet);
	vkcomponent::DescriptorBuilder::Begin(p_renderer->GetDescriptorLayoutCache(), p_renderer->GetDescriptorAllocator())
	.BindImage(1, &imageBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
	.Build(depthTextureSet);
}

void VulkanOffscreen::BeginOffscreenRenderpass()
{
	VkClearValue clearValues[1];
	clearValues[0].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkinit::RenderpassBeginInfo(shadow.shadowPass, shadow.shadowExtent, VK_NULL_HANDLE);
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;

	VkViewport viewport = {};
	viewport.width = (float)shadow.shadowExtent.width;
	viewport.height = (float)shadow.shadowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(p_renderer->GetCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = {0,0};
	scissor.extent = shadow.shadowExtent;
	vkCmdSetScissor(p_renderer->GetCommandBuffer(), 0, 1, &scissor);

	// One pass per cascade
	// The layer that this pass renders to is defined by the cascade's image view (selected via the cascade's descriptor set)
	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
		renderPassBeginInfo.framebuffer = cascades[i].frameBuffer;
	}
	vkCmdBeginRenderPass(p_renderer->GetCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanOffscreen::debugShadows(bool debug /*= false*/)
{
	if(debug == true)
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
		vkCmdBindDescriptorSets(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebugLayout, 0, 1, &debugTextureSet, 0, nullptr);
		vkCmdBindPipeline(p_renderer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, shadowDebug);
		PushConstBlock pushConstBlock = {};
		vkCmdPushConstants(p_renderer->GetCommandBuffer(), shadowDebugLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);
		vkCmdDraw(p_renderer->GetCommandBuffer(), 3, 1, 0, 0);
	}

}

void VulkanOffscreen::drawOffscreenShadows(const std::vector<RenderObject>& objects)
{
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)shadow.shadowExtent.width;
	viewport.height = (float)shadow.shadowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkExtent2D imageExtent;
	imageExtent.width = shadow.shadowExtent.width;
	imageExtent.height = shadow.shadowExtent.height;
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = imageExtent;

	vkCmdSetViewport(p_renderer->GetCommandBuffer(), 0, 1, &viewport);
	vkCmdSetScissor(p_renderer->GetCommandBuffer(), 0, 1, &scissor);

	DescriptorSetData descriptorSets;
	descriptorSets.uniform = GetCurrentFrame().globalDescriptor;
	descriptorSets.uniformOffset = 0;

	std::vector<DrawCall> drawCalls = BatchDrawCalls(objects, descriptorSets);

	UploadLightData(p_renderer->GetAllocator(), GetCurrentFrame().cameraBuffer.allocation, cascades);
	IssueDrawCalls(p_renderer->GetCommandBuffer(), GetCurrentFrame().indirectDrawBuffer.buffer,drawCalls, shadow.shadowPipeline, shadow.shadowPipelineLayout);
}

void VulkanOffscreen::EndOffscreenRenderpass()
{
	vkCmdEndRenderPass(p_renderer->GetCommandBuffer());
}