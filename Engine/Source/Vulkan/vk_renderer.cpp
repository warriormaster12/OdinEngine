#include "Include/vk_renderer.h"
#include "vk_device.h"
#include "Include/vk_init.h"
#include "vk_utils.h"
#include "vk_check.h"
#include "vk_functions.h"
#include "vk_shaderhandler.h"
#include "vk_textures.h"
#include "asset_builder.h"
#include "imgui_impl_vulkan.h"
#include "Imgui_layer.h"






#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
//bootstrap library
#include "VkBootstrap.h"

#include "logger.h"

vkcomponent::PipelineBuilder pipelineBuilder;
uint32_t swapchainImageIndex;
VkCommandBuffer cmd;
VkResult drawResult;


// Utility (pure) functions are put in an anonymous namespace

namespace {
    void UploadCameraData(const VmaAllocator& allocator, const VmaAllocation& allocation, const Camera& cam, const std::array<Cascade, SHADOW_MAP_CASCADE_COUNT>& cascades)
    {
		
        GPUCameraData camData;
        camData.view = cam.GetViewMatrix();
        camData.proj = cam.GetProjectionMatrix(false);
        camData.viewproj = camData.proj * camData.view;
        camData.camPos = glm::vec4(glm::vec3(cam.position), 0.0f);

		for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			camData.cascadeData.cascadeViewProjMat[i] = cascades[i].viewProjMatrix;
			camData.cascadeData.cascadeSplits[i] = cascades[i].splitDepth;
		}
		camData.cascadeData.cascadeSplitsDebug = 1;

		UploadSingleData(allocator, allocation, camData);
    }

    void UploadSceneData(const VmaAllocator& allocator, const VmaAllocation& allocation, const GPUSceneData& data, size_t offset)
    {
		UploadSingleData(allocator, allocation, data, offset);
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

    void UploadMaterialData(const VmaAllocator& allocator, std::vector<Material>& materials)
    {
        for(auto material : materials)
        {
			if(material.isOutdated == true)
			{

				GPUMaterialData materialData;
				materialData.albedo = material.albedo;
				materialData.metallic = glm::vec4(glm::vec3(material.metallic), 1.0f);
				materialData.roughness = glm::vec4(glm::vec3(material.roughness), 1.0f);
				materialData.ao = glm::vec4(glm::vec3(material.ao), 1.0f);
				materialData.emissionColor = glm::vec4(glm::vec3(material.emissionColor),1.0f);
				materialData.emissionPower = glm::vec4(material.emissionPower);

				UploadSingleData(allocator, material.buffer.allocation, materialData);
				material.isOutdated = false;
			}
        }
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
				dc.position = objects[i].position;
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

    void BindDynamicStates()
    {
        vkCmdSetViewport(cmd, 0, 1, &pipelineBuilder.viewport);
        vkCmdSetScissor(cmd, 0, 1, &pipelineBuilder.scissor);
    }

    void BindMaterial(Material* material, const DescriptorSetData& descriptorSets)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->materialPass.pipeline);

        //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipelineLayout, 0, 1, &descriptorSets.uniform, 1, &descriptorSets.uniformOffset);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->materialPass.layout, 0, 1, &descriptorSets.uniform, 0, nullptr);

        //object data descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->materialPass.layout, 1, 1, &descriptorSets.object, 0, &descriptorSets.objectOffset);
        //texture + material descriptor
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, material->materialPass.layout, 2, 1, &material->materialSet, 0, nullptr);
    }

    void BindMesh(Mesh* mesh)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vertexBuffer.buffer, &offset);
    }

	void IssueDrawCalls(const VkCommandBuffer& cmd, const VkBuffer& drawCommandBuffer, const std::vector<DrawCall>& drawCalls)
	{
		PushConstBlock pushConstBlock = {};
		for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			
			pushConstBlock.cascadeIndex = 0;
			
		}
		for (const DrawCall& dc : drawCalls)
		{
			
			pushConstBlock.position = glm::vec4(glm::vec3(dc.position),0.0f);
			
			vkCmdPushConstants(cmd,dc.pMaterial->materialPass.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

			BindDynamicStates();

			BindMaterial(dc.pMaterial, dc.descriptorSets);
			BindMesh(dc.pMesh);

			uint32_t stride = sizeof(VkDrawIndirectCommand);
			uint32_t offset = dc.index * stride;

			vkCmdDrawIndirect(cmd, drawCommandBuffer, offset, dc.count, stride);
		}
	}
}



VkCommandBuffer& VulkanRenderer::GetCommandBuffer()
{
	return cmd;
}

void VulkanRenderer::Init(WindowHandler& windowHandler)
{
	p_windowHandler = &windowHandler;
	VkDeviceManager::InitVulkan(swapChainObj, windowHandler);
	InitSwapchain();
	InitDefaultRenderpass();
	InitFramebuffers();
	InitCommands();
	InitSyncStructures();
	InitDescriptors();
	LoadImage("");
	InitSamplers();
	offscreen.InitOffscreen(*this);
	InitPipelines();


	camera.position = { 0.f,0.f,10.f };

	ENGINE_CORE_INFO("vulkan intialized");
}

void VulkanRenderer::CleanUp()
{	
	vkDeviceWaitIdle(VkDeviceManager::GetDevice());

	swapDeletionQueue.Flush();
	mainDeletionQueue.Flush();

	vkDestroySurfaceKHR(VkDeviceManager::GetInstance(), swapChainObj.surface, nullptr);

	vmaDestroyAllocator(VkDeviceManager::GetAllocator());
	vkDestroyDevice(VkDeviceManager::GetDevice(), nullptr);
	vkb::destroy_debug_utils_messenger(VkDeviceManager::GetInstance(), VkDeviceManager::GetDebugMessenger());
	vkDestroyInstance(VkDeviceManager::GetInstance(), nullptr);

	ENGINE_CORE_INFO("vulkan destroyed");
}


void VulkanRenderer::BeginCommands()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(VkDeviceManager::GetDevice(), 1, &GetCurrentFrame().renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(VkDeviceManager::GetDevice(), 1, &GetCurrentFrame().renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().mainCommandBuffer, 0));

	//request image from the swapchain
	drawResult = vkAcquireNextImageKHR(VkDeviceManager::GetDevice(), swapChainObj.swapchain, 1000000000, GetCurrentFrame().presentSemaphore, nullptr, &swapchainImageIndex);
	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
	} else if (drawResult != VK_SUCCESS && drawResult != VK_SUBOPTIMAL_KHR) {
		ENGINE_CORE_ERROR("failed to acquire swap chain image!");
		abort();
	}

	//naming it cmd for shorter writing
	cmd = GetCurrentFrame().mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


}

void VulkanRenderer::BeginRenderpass()
{
	VkClearValue clearValue;
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;
	
	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(renderPass, swapChainObj.actualExtent, framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];
	
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	
}

void VulkanRenderer::EndRenderpass()
{
	//finalize the render pass
	vkCmdEndRenderPass(cmd);
}

void VulkanRenderer::EndCommands()
{
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &GetCurrentFrame().presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &GetCurrentFrame().renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(VkDeviceManager::GetGraphicsQueue(), 1, &submit, GetCurrentFrame().renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();

	presentInfo.pSwapchains = &swapChainObj.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	drawResult = vkQueuePresentKHR(VkDeviceManager::GetGraphicsQueue(), &presentInfo);

	if (drawResult == VK_ERROR_OUT_OF_DATE_KHR || drawResult == VK_SUBOPTIMAL_KHR || p_windowHandler->frameBufferResized == true) {
		p_windowHandler->frameBufferResized = false;
		RecreateSwapchain();
	} else if (drawResult != VK_SUCCESS) {
		ENGINE_CORE_ERROR("failed to present swap chain image!");
		abort();
	}


	//increase the number of frames drawn
	frameNumber++;
}

void VulkanRenderer::DrawObjects(const std::vector<RenderObject>& objects)
{
	size_t frameIndex = frameNumber % FRAME_OVERLAP;
	//size_t uniformOffset = PadUniformBufferSize(sizeof(GPUSceneData)) * frameIndex;
	size_t uniformOffset = sizeof(GPUSceneData) * frameIndex;

	// Static light data, can be moved away
	//TODO: make proper pointlight, spotlight and directional light
	
	sceneParameters.dLight.intensity = glm::vec4(10.0f);
	sceneParameters.dLight.color = glm::vec4(1.0f);
	sceneParameters.dLight.direction = glm::vec4(glm::vec3(glm::normalize(offscreen.GetLightPos())), 0.0f);
	sceneParameters.plightCount = glm::vec4(3);
	sceneParameters.pointLights[0].intensity = glm::vec4(100.0f);
	sceneParameters.pointLights[0].position = glm::vec4(glm::vec3(0.0f,  5.0f, -3.0f),1.0f);
	sceneParameters.pointLights[0].color = glm::vec4(glm::vec3(1.0f,1.0f,1.0f),1.0f);
	sceneParameters.pointLights[0].radius = glm::vec4(10.0f);
	sceneParameters.pointLights[1].intensity = glm::vec4(100.0f);
	sceneParameters.pointLights[1].position = glm::vec4(glm::vec3(0.0f,  4.0f, 7.0f),1.0f);
	sceneParameters.pointLights[1].color = glm::vec4(glm::vec3(1.0f,0.0f,0.0f),1.0f);
	sceneParameters.pointLights[1].radius = glm::vec4(5.0f);
	sceneParameters.pointLights[2].intensity = glm::vec4(100.0f);
	sceneParameters.pointLights[2].position = glm::vec4(glm::vec3(4.0f,  1.0f, 7.0f),1.0f);
	sceneParameters.pointLights[2].color = glm::vec4(glm::vec3(0.0f,0.0f,1.0f),1.0f);
	sceneParameters.pointLights[2].radius = glm::vec4(15.0f);

	// Convert material ID to material
	// TODO: Handle nullptr material, apply default?
	std::vector<Material> materials;
	materials.reserve(materialList.size());
	for (const std::string& materialName : materialList)
		materials.push_back(*GetMaterial(materialName));

	// Store descriptor set data
	DescriptorSetData descriptorSets;
	descriptorSets.uniform = GetCurrentFrame().globalDescriptor;
	descriptorSets.uniformOffset = 0;
	descriptorSets.object = GetCurrentFrame().objectDescriptor;
	descriptorSets.objectOffset = 0;

	std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> m_cascades;
	for(int i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
	{
		m_cascades[i] = offscreen.GetCurrenCascade(i);
	}

	UploadCameraData(VkDeviceManager::GetAllocator(), GetCurrentFrame().cameraBuffer.allocation, camera, m_cascades);
	UploadSceneData(VkDeviceManager::GetAllocator(), sceneParameterBuffer.allocation, sceneParameters, uniformOffset);
	UploadObjectData(VkDeviceManager::GetAllocator(), GetCurrentFrame().objectBuffer.allocation, objects);
	
	UploadMaterialData(VkDeviceManager::GetAllocator(), materials);

	UploadDrawCalls(VkDeviceManager::GetAllocator(), GetCurrentFrame().indirectDrawBuffer.allocation, objects);

	std::vector<DrawCall> drawCalls = BatchDrawCalls(objects, descriptorSets);
	IssueDrawCalls(cmd, GetCurrentFrame().indirectDrawBuffer.buffer, drawCalls);
}

void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(uploadContext.commandPool, 1);

    VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(VkDeviceManager::GetDevice(), &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(VkDeviceManager::GetGraphicsQueue(), 1, &submit, uploadContext.uploadFence));

	vkWaitForFences(VkDeviceManager::GetDevice(), 1, &uploadContext.uploadFence, true, 9999999999);
	vkResetFences(VkDeviceManager::GetDevice(), 1, &uploadContext.uploadFence);

    //clear the command pool. This will free the command buffer too
	vkResetCommandPool(VkDeviceManager::GetDevice(), uploadContext.commandPool, 0);
}

void VulkanRenderer::EnqueueCleanup(std::function<void()>&& function, FunctionQueuer* p_override /*= nullptr*/)
{
	if (p_override == nullptr)
	{
		mainDeletionQueue.PushFunction([=]() { function(); });
	}
	else
	{
		p_override->PushFunction([=]() { function(); });
	}
}

void VulkanRenderer::UploadMesh(Mesh& mesh)
{
	const size_t BUFFER_SIZE = mesh.vertices.size() * sizeof(Vertex);

	AllocatedBuffer stagingBuffer;
	{
		CreateBufferInfo info;
		info.allocSize = BUFFER_SIZE;
		info.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &stagingBuffer, info);
	}

	//copy vertex data
	UploadVectorData(VkDeviceManager::GetAllocator(), stagingBuffer.allocation, mesh.vertices);

	{
		CreateBufferInfo info;
		info.allocSize = BUFFER_SIZE;
		info.bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		CreateBuffer(VkDeviceManager::GetAllocator(), &mesh.vertexBuffer, info);
	}

	//add the destruction of triangle mesh buffer to the deletion queue
	EnqueueCleanup([=]() {
		vmaDestroyBuffer(VkDeviceManager::GetAllocator(), mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
	});

	ImmediateSubmit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = BUFFER_SIZE;
		vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, & copy);
	});

	vmaDestroyBuffer(VkDeviceManager::GetAllocator(), stagingBuffer.buffer, stagingBuffer.allocation);
}

void VulkanRenderer::CreateMaterial(vkcomponent::ShaderPass* inputPass, const std::string& name)
{
	Material mat;
	mat.materialPass = *inputPass;
	materials[name] = mat;
	materialList.push_back(name);
	p_descriptorAllocator->AllocateVariableSet(&materials[name].materialSet, materialTextureSetLayout, mat.textures.size());
	

	{
		CreateBufferInfo info;
		info.allocSize = sizeof(GPUMaterialData);
		info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		CreateBuffer(VkDeviceManager::GetAllocator(), &materials[name].buffer, info);
	}

	VkDescriptorBufferInfo materialBufferInfo;
	materialBufferInfo.buffer = materials[name].buffer.buffer;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = sizeof(GPUMaterialData);

	VkWriteDescriptorSet objectFragWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, materials[name].materialSet, &materialBufferInfo, 0);
	vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &objectFragWrite, 0, nullptr);
	
	CreateTextures(name, mat.textures);

	materials[name].isOutdated = true;

	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = offscreen.GetDepthImage().sampler;
	imageBufferInfo.imageView = offscreen.GetDepthImage().depthImage.imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet shadowImage = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, materials[name].materialSet, &imageBufferInfo, 1);
	vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &shadowImage, 0, nullptr);
	

	EnqueueCleanup([=]() {
		vmaDestroyBuffer(VkDeviceManager::GetAllocator(), materials[name].buffer.buffer, materials[name].buffer.allocation);
	});
}

Material* VulkanRenderer::GetMaterial(const std::string& name)
{
	//search for the object, and return nullpointer if not found
	auto it = materials.find(name);
	if (it == materials.end()) {
		return nullptr;
	}
	else {
		return &it->second;
	}
}

bool VulkanRenderer::LoadComputeShader(const std::string& shaderPath, VkPipeline& pipeline, VkPipelineLayout& layout, std::vector<VkDescriptorSetLayout>& descriptorLayouts)
{
	vkcomponent::ShaderModule computeModule;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(shaderPath).c_str(), &computeModule, VkDeviceManager::GetDevice());

	vkcomponent::ShaderEffect* computeEffect = new vkcomponent::ShaderEffect();
	computeEffect->AddStage(&computeModule, VK_SHADER_STAGE_COMPUTE_BIT);

	VkPipelineLayoutCreateInfo computePipInfo = vkinit::PipelineLayoutCreateInfo();
	computePipInfo.pSetLayouts = descriptorLayouts.data();
	computePipInfo.setLayoutCount = descriptorLayouts.size();
	std::vector<vkcomponent::ShaderModule> modules = {computeModule};
	computeEffect = vkcomponent::BuildEffect(modules, computePipInfo);

	vkcomponent::ComputePipelineBuilder computeBuilder;
	computeBuilder.pipelineLayout = computeEffect->builtLayout;
	computeBuilder.shaderStage = vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeModule.module);


	layout = computeEffect->builtLayout;
	pipeline = computeBuilder.BuildPipeline();

	vkDestroyShaderModule(VkDeviceManager::GetDevice(), computeModule.module, nullptr);

	EnqueueCleanup([=]() {
		vkDestroyPipeline(VkDeviceManager::GetDevice(), pipeline, nullptr);
		vkDestroyPipelineLayout(VkDeviceManager::GetDevice(), layout, nullptr);
	});

	return true;
}

void VulkanRenderer::CreateTextures(const std::string& materialName, std::vector<std::string>& texturePaths)
{
	std::vector<VkFormat> formats = {
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8G8B8A8_UNORM,
	};
	Material* texturedMaterial = GetMaterial(materialName);
	if (texturedMaterial == nullptr)
	{
		ENGINE_CORE_ERROR("Could not load texture for non-existent material: {}", materialName);
		return;
	}
	VkDescriptorImageInfo imageBufferInfo[texturePaths.size()];
	for(int i = 0; i < texturePaths.size(); i++)
	{
		std::filesystem::path textureNamePath = texturePaths[i]; 
		const std::string processedName = textureNamePath.stem().u8string();
		if(texturePaths[i] != "")
		{
			LoadImage(texturePaths[i], formats[i]);

			//write to the descriptor set so that it points to our input texture
			imageBufferInfo[i].sampler = textureSampler;
			imageBufferInfo[i].imageView = loadedTextures[processedName].imageView;
			imageBufferInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else
		{
			imageBufferInfo[i].sampler = textureSampler;
			imageBufferInfo[i].imageView = loadedTextures["empty"].imageView;
			imageBufferInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}
	// +1: binding 0 is used for material data (uniform data)
	VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMaterial->materialSet, imageBufferInfo, 2, texturePaths.size());
	
	vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputTexture, 0, nullptr);
}

/* Private functions */



void VulkanRenderer::InitSwapchain()
{
	swapChainObj.InitSwapchain(p_windowHandler->p_window, swapDeletionQueue);
}

void VulkanRenderer::InitDefaultRenderpass()
{
	//we define an attachment description for our main color image
	//the attachment is loaded as "clear" when renderpass start
	//the attachment is stored when renderpass ends
	//the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
	//we dont care about stencil, and dont use multisampling

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swapChainObj.swapchainImageFormat;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = swapChainObj.depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
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


	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//2 attachments from said array
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = &attachments[0];
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;
	
	VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &render_pass_info, nullptr, &renderPass));
	EnqueueCleanup([=]() {
		vkDestroyRenderPass(VkDeviceManager::GetDevice(), renderPass, nullptr);
	});
}



void VulkanRenderer::InitFramebuffers()
{
	const uint32_t swapchainImageCount = swapChainObj.swapchainImageViews.size();
	framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);
	for (int i = 0; i < swapchainImageCount; i++) {
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		VkFramebufferCreateInfo fb_info = vkinit::FramebufferCreateInfo(renderPass, swapChainObj.actualExtent);
		std::array <VkImageView, 2> attachments = {swapChainObj.swapchainImageViews[i], swapChainObj.depthImageView};


		fb_info.attachmentCount = attachments.size();
		fb_info.pAttachments = attachments.data();
		VK_CHECK(vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fb_info, nullptr, &framebuffers[i]));
		EnqueueCleanup([=]() {
			vkDestroyFramebuffer(VkDeviceManager::GetDevice(), framebuffers[i], nullptr);
		},&swapDeletionQueue);
	}
	
}

void VulkanRenderer::InitCommands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(VkDeviceManager::GetGraphicsQueueFamily(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++) {

	
		VK_CHECK(vkCreateCommandPool(VkDeviceManager::GetDevice(), &commandPoolInfo, nullptr, &frames[i].commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(frames[i].commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(VkDeviceManager::GetDevice(), &cmdAllocInfo, &frames[i].mainCommandBuffer));
		EnqueueCleanup([=]() {
			vkDestroyCommandPool(VkDeviceManager::GetDevice(), frames[i].commandPool, nullptr);
		});
	}
	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(VkDeviceManager::GetGraphicsQueueFamily());
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(VkDeviceManager::GetDevice(), &uploadCommandPoolInfo, nullptr, &uploadContext.commandPool));
	EnqueueCleanup([=]() {
		vkDestroyCommandPool(VkDeviceManager::GetDevice(), uploadContext.commandPool, nullptr);
	});
}

void VulkanRenderer::InitSyncStructures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		VK_CHECK(vkCreateFence(VkDeviceManager::GetDevice(), &fenceCreateInfo, nullptr, &frames[i].renderFence));

		//enqueue the destruction of the fence
		EnqueueCleanup([=]() {
			vkDestroyFence(VkDeviceManager::GetDevice(), frames[i].renderFence, nullptr);
			});


		VK_CHECK(vkCreateSemaphore(VkDeviceManager::GetDevice(), &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore));
		VK_CHECK(vkCreateSemaphore(VkDeviceManager::GetDevice(), &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore));

		//enqueue the destruction of semaphores
		EnqueueCleanup([=]() {
			vkDestroySemaphore(VkDeviceManager::GetDevice(), frames[i].presentSemaphore, nullptr);
			vkDestroySemaphore(VkDeviceManager::GetDevice(), frames[i].renderSemaphore, nullptr);
			});
		
	}
	 VkFenceCreateInfo uploadFenceCreateInfo = vkinit::FenceCreateInfo();

	VK_CHECK(vkCreateFence(VkDeviceManager::GetDevice(), &uploadFenceCreateInfo, nullptr, &uploadContext.uploadFence));

	EnqueueCleanup([=]() {
		vkDestroyFence(VkDeviceManager::GetDevice(), uploadContext.uploadFence, nullptr);
	});

}

void VulkanRenderer::InitDescriptors()
{
	p_descriptorAllocator = new vkcomponent::DescriptorAllocator{};
	p_descriptorAllocator->Init(VkDeviceManager::GetDevice());

	p_descriptorLayoutCache = new vkcomponent::DescriptorLayoutCache{};
	p_descriptorLayoutCache->Init(VkDeviceManager::GetDevice());
	
	VkDescriptorSetLayoutBinding cameraBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0);
	//VkDescriptorSetLayoutBinding sceneBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	std::vector<VkDescriptorSetLayoutBinding> bindings = { cameraBind,sceneBind };
	VkDescriptorSetLayoutCreateInfo _set1 = vkinit::DescriptorLayoutInfo(bindings);
	globalSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set1);

	VkDescriptorSetLayoutBinding objectBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	std::vector<VkDescriptorSetLayoutBinding> objectBindings = { objectBind };
	VkDescriptorSetLayoutCreateInfo _set2 = vkinit::DescriptorLayoutInfo(objectBindings);
	objectSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set2);


	VkDescriptorSetLayoutBinding objectMaterialBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding shadowBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding TexturesBind = vkinit::DescriptorsetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 7);
	
	
	std::vector<VkDescriptorSetLayoutBinding> textureBindings = {objectMaterialBind, shadowBind, TexturesBind};
	std::vector<VkDescriptorBindingFlagsEXT> flags = {0, 0, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT};
	auto& info = vkinit::DescriptorLayoutBindingFlagsInfo(flags);
	VkDescriptorSetLayoutCreateInfo _set3 = vkinit::DescriptorLayoutInfo(textureBindings, &info);
	materialTextureSetLayout = p_descriptorLayoutCache->CreateDescriptorLayout(&_set3);

	{
		CreateBufferInfo info;
		//info.allocSize = FRAME_OVERLAP * PadUniformBufferSize(sizeof(GPUSceneData));
		info.allocSize = FRAME_OVERLAP * sizeof(GPUSceneData);
		info.bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		CreateBuffer(VkDeviceManager::GetAllocator(), &sceneParameterBuffer, info);
	}

    const int MAX_OBJECTS = 10000;
    const int MAX_COMMANDS = 1000;

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		frames[i].p_dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		frames[i].p_dynamicDescriptorAllocator->Init(VkDeviceManager::GetDevice());
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].globalDescriptor, globalSetLayout);
		frames[i].p_dynamicDescriptorAllocator->Allocate(&frames[i].objectDescriptor, objectSetLayout);

		{
			CreateBufferInfo info;
			info.allocSize = sizeof(GPUCameraData);
			info.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(VkDeviceManager::GetAllocator(), &frames[i].cameraBuffer, info);
		}

		{
			CreateBufferInfo info;
			info.allocSize = MAX_OBJECTS * sizeof(GPUObjectData);
			info.bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            CreateBuffer(VkDeviceManager::GetAllocator(), &frames[i].objectBuffer, info);
		}

		{
			CreateBufferInfo info;
			info.allocSize = MAX_COMMANDS * sizeof(VkDrawIndirectCommand);
			info.bufferUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			CreateBuffer(VkDeviceManager::GetAllocator(), &frames[i].indirectDrawBuffer, info);
		}

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = frames[i].cameraBuffer.buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = sceneParameterBuffer.buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = frames[i].objectBuffer.buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

		vkcomponent::DescriptorBuilder::Begin(p_descriptorLayoutCache, frames[i].p_dynamicDescriptorAllocator)
		.BindBuffer(0, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		.BindBuffer(1, &sceneInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build(frames[i].globalDescriptor);

		vkcomponent::DescriptorBuilder::Begin(p_descriptorLayoutCache, frames[i].p_dynamicDescriptorAllocator)
		.BindBuffer(0, &objectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(frames[i].objectDescriptor);

		EnqueueCleanup([=]()
		{
			vmaDestroyBuffer(VkDeviceManager::GetAllocator(), frames[i].objectBuffer.buffer, frames[i].objectBuffer.allocation);
			vmaDestroyBuffer(VkDeviceManager::GetAllocator(), frames[i].indirectDrawBuffer.buffer, frames[i].indirectDrawBuffer.allocation);
			vmaDestroyBuffer(VkDeviceManager::GetAllocator(), frames[i].cameraBuffer.buffer, frames[i].cameraBuffer.allocation);
		});
	}

	EnqueueCleanup([=]() {
		vmaDestroyBuffer(VkDeviceManager::GetAllocator(), sceneParameterBuffer.buffer, sceneParameterBuffer.allocation);
		for (auto& frame : frames)
		{
			frame.p_dynamicDescriptorAllocator->CleanUp();
		}
		p_descriptorAllocator->CleanUp();
		p_descriptorLayoutCache->CleanUp();
	});
	
}

void VulkanRenderer::InitSamplers()
{
	//create a sampler for the texture
	VkSamplerCreateInfo samplerInfo = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST);

	vkCreateSampler(VkDeviceManager::GetDevice(), &samplerInfo, nullptr, &textureSampler);

	EnqueueCleanup([=]() {
		vkDestroySampler(VkDeviceManager::GetDevice(), textureSampler, nullptr);
	});
}

void VulkanRenderer::InitPipelines()
{
	//VkShaderModule texturedMeshShader;
	vkcomponent::ShaderModule texturedMeshShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_pbr_lit.frag").c_str(), &texturedMeshShader, VkDeviceManager::GetDevice());

	//VkShaderModule meshVertShader;
	vkcomponent::ShaderModule meshVertShader;
	vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/mesh_triangle.vert").c_str(), &meshVertShader, VkDeviceManager::GetDevice());

	vkcomponent::ShaderEffect* defaultEffect = new vkcomponent::ShaderEffect();
	std::vector<vkcomponent::ShaderModule> shaderModules = {meshVertShader, texturedMeshShader};
	std::array<VkDescriptorSetLayout, 3> layouts= {globalSetLayout, objectSetLayout, materialTextureSetLayout};
	VkPipelineLayoutCreateInfo meshpipInfo = vkinit::PipelineLayoutCreateInfo();
	VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(PushConstBlock);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	meshpipInfo.pSetLayouts = layouts.data();
	meshpipInfo.setLayoutCount = layouts.size();
	meshpipInfo.pushConstantRangeCount = 1;
	meshpipInfo.pPushConstantRanges = &pushConstant;
	defaultEffect = vkcomponent::BuildEffect(shaderModules, meshpipInfo);

	//hook the push constants layout
	pipelineBuilder.pipelineLayout = defaultEffect->builtLayout;
	//we have copied layout to builder so now we can flush old one
	defaultEffect->FlushLayout();

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder.viewport.x = 0.0f;
	pipelineBuilder.viewport.y = 0.0f;
	pipelineBuilder.viewport.width = (float)swapChainObj.actualExtent.width;
	pipelineBuilder.viewport.height = (float)swapChainObj.actualExtent.height;
	pipelineBuilder.viewport.minDepth = 0.0f;
	pipelineBuilder.viewport.maxDepth = 1.0f;

	pipelineBuilder.blendConstant = {0.0f, 0.0f, 0.0f, 0.0f};

	pipelineBuilder.scissor.offset = { 0, 0 };
	pipelineBuilder.scissor.extent = swapChainObj.actualExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();


	//default depthtesting
	pipelineBuilder.depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the mesh pipeline
	std::vector <LocationInfo> locations = {{VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position)},
		{VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normal)},
		{VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color)},
		{VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, uv)}
	};
	VertexInputDescription vertexDescription = Vertex::GetVertexDescription(locations);

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	
	//build the mesh triangle pipeline
	vkcomponent::ShaderPass* defaultPass = vkcomponent::BuildShader(renderPass, pipelineBuilder, defaultEffect);
	
	CreateMaterial(defaultPass, "defaultMat");

	//VkShaderModule skyFragShader;
	// vkcomponent::ShaderModule skyFragShader;
	// vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/skybox_frag.frag").c_str(), &skyFragShader, device);

	// //VkShaderModule skyVertShader;
	// vkcomponent::ShaderModule skyVertShader;
	// vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(".Shaders/skybox_vert.vert").c_str(), &skyVertShader, device);

	// vkcomponent::ShaderEffect* skyEffect = new vkcomponent::ShaderEffect();
	// std::vector<vkcomponent::ShaderModule> skyShaderModules = {skyVertShader, skyFragShader};
	// std::array<VkDescriptorSetLayout, 1> skyLayouts= {globalSetLayout};
	// VkPipelineLayoutCreateInfo skypipInfo = meshpipInfo;
	// meshpipInfo.pSetLayouts = skyLayouts.data();
	// meshpipInfo.setLayoutCount = skyLayouts.size();
	// skyEffect = vkcomponent::BuildEffect(device, skyShaderModules, skypipInfo);

	// //hook the push constants layout
	// pipelineBuilder.pipelineLayout = skyEffect->builtLayout;
	// //we have copied layout to builder so now we can flush old one
	// skyEffect->FlushLayout();
	// pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	// locations.resize(1);
	// vertexDescription = Vertex::GetVertexDescription(locations);

	// //connect the pipeline builder vertex input info to the one we get from Vertex
	// pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	// pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	// pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	// pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	// vkcomponent::ShaderPass* skyPass = vkcomponent::BuildShader(device, renderPass, pipelineBuilder, skyEffect);

	// CreateMaterial(skyPass, "skyMat");


	//currently only testing if creating compute pipeline works
	// VkPipeline computePip;
	// VkPipelineLayout computePipLayout;
	// std::vector<VkDescriptorSetLayout> computeLayouts = {globalSetLayout};
	// LoadComputeShader(".Shaders/indirect_cull.comp", computePip, computePipLayout, computeLayouts);


	EnqueueCleanup([=]() {
		defaultPass->FlushPass();
		//skyPass->FlushPass(device);
	});
}

void VulkanRenderer::RecreateSwapchain()
{	
	vkDeviceWaitIdle(VkDeviceManager::GetDevice());
	swapDeletionQueue.Flush();
	int width = 0, height = 0;
	glfwGetFramebufferSize(p_windowHandler->p_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(p_windowHandler->p_window, &width, &height);
		glfwWaitEvents();
	}

	InitSwapchain();
	InitFramebuffers();	

	//update parts of the pipeline that change dynamically
	pipelineBuilder.viewport.width = (float)swapChainObj.actualExtent.width;
	pipelineBuilder.viewport.height = (float)swapChainObj.actualExtent.height;

	pipelineBuilder.scissor.extent = swapChainObj.actualExtent;
}

void VulkanRenderer::LoadImage(const std::string& texturePath, const VkFormat& imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{
	Texture inputTextures;
	const std::filesystem::path path = texturePath;
	const std::filesystem::path bin_path = texturePath + ".bin";
	assets::TextureInfo textureInfo;
	if (imageFormat == VK_FORMAT_R8G8B8A8_SRGB)
	{
		textureInfo.textureFormat = assets::TextureFormat::RGBA8;
		asset_builder::ConvertImage(path,bin_path, textureInfo);
	}
	if(imageFormat == VK_FORMAT_R8G8B8A8_UNORM)
	{
		textureInfo.textureFormat = assets::TextureFormat::UNORM8;
		asset_builder::ConvertImage(path,bin_path, textureInfo);
	}

	vkcomponent::LoadImageFromAsset(*this, (texturePath + ".bin").c_str(), &inputTextures.image);

	VkImageViewCreateInfo imageinfo = vkinit::ImageViewCreateInfo(imageFormat, inputTextures.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(VkDeviceManager::GetDevice(), &imageinfo, nullptr, &inputTextures.imageView);
	if(texturePath != "")
	{
		std::filesystem::path textureNamePath = texturePath; 
		const std::string processedName = textureNamePath.stem().u8string();

		loadedTextures[processedName] = inputTextures;
	}
	else
	{
		loadedTextures["empty"] = inputTextures;
	}

	EnqueueCleanup([=]() {
		vkDestroyImageView(VkDeviceManager::GetDevice(), inputTextures.imageView, nullptr);
		vmaDestroyImage(VkDeviceManager::GetAllocator(), inputTextures.image.image, inputTextures.image.allocation);
	});
}

