#include "Include/vk_renderer.h"
#include "Include/vk_types.h"
#include "Include/vk_init.h"
#include "vk_components/Include/vk_check.h"
#include "vk_components/Include/vk_pipelinebuilder.h"
#include "vk_components/Include/vk_shaderhandler.h"
#include "vk_components/Include/vk_textures.h"
#include "../Asset_manager/Include/asset_builder.h"
#include "../third-party/imgui/Include/imgui_impl_vulkan.h"
#include "../Editor/Include/Imgui_layer.h"




//bootstrap library
#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "../third-party/Vma/vk_mem_alloc.h"

#include "../Logger/Include/Logger.h"

vkcomponent::PipelineBuilder pipelineBuilder;

constexpr bool bUseValidationLayers = true;
void VulkanRenderer::init(WindowHandler& windowHandler)
{
	_windowHandler = &windowHandler;
	init_vulkan();
	_swapChainObj.init_swapchain(_windowHandler->_window);

	init_default_renderpass();

	init_framebuffers();

	init_commands();

	init_sync_structures();

	init_descriptors();

	init_pipelines();

	load_images();	

	load_meshes();

	init_scene();

	_camera.position = { 0.f,0.f,10.f };

	ENGINE_CORE_INFO("vulkan intialized");
}
void VulkanRenderer::cleanup()
{	
		
	vkDeviceWaitIdle(_device);
	
	_swapDeletionQueue.flush();
	_mainDeletionQueue.flush();

	vkDestroySurfaceKHR(_instance, _swapChainObj._surface, nullptr);

	vmaDestroyAllocator(_allocator);
	vkDestroyDevice(_device, nullptr);
	vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
	vkDestroyInstance(_instance, nullptr);

	ENGINE_CORE_INFO("vulkan destroyed");
}

void VulkanRenderer::frameBufferResize()
{
	frameBufferResized = true;
}

void VulkanRenderer::draw()
{
	//wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &get_current_frame()._renderFence));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult result = vkAcquireNextImageKHR(_device, _swapChainObj._swapchain, 1000000000, get_current_frame()._presentSemaphore, nullptr, &swapchainImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearValue clearValue;
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass. 
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass, _swapChainObj._actualExtent, _framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;

	VkClearValue clearValues[] = { clearValue, depthClear };

	rpInfo.pClearValues = &clearValues[0];
	
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_objects(cmd, _renderables.data(), _renderables.size());	
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = vkinit::submit_info(&cmd);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::present_info();

	presentInfo.pSwapchains = &_swapChainObj._swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized == true) {
		frameBufferResized = false;
		recreate_swapchain();
	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}


	//increase the number of frames drawn
	_frameNumber++;
}

void VulkanRenderer::run()
{
	draw();
}

FrameData& VulkanRenderer::get_current_frame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}


FrameData& VulkanRenderer::get_last_frame()
{
	return _frames[(_frameNumber -1) % 2];
}


void VulkanRenderer::init_vulkan()
{
	vkb::InstanceBuilder builder;

	//make the vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 1, 0)		
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//grab the instance 
	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;

	SDL_Vulkan_CreateSurface(_windowHandler->_window, _instance, &_swapChainObj._surface);
	

	//use vkbootstrap to select a gpu. 
	//We want a gpu that can write to the SDL surface and supports vulkan 1.2
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(_swapChainObj._surface)
		.select()
		.value();
	//create the final vulkan device

	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();
	// Get the VkDevice handle used in the rest of a vulkan application
	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

	_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = _chosenGPU;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);

	

	vkGetPhysicalDeviceProperties(_chosenGPU, &_gpuProperties);
	ENGINE_CORE_INFO(physicalDevice.properties.deviceName);
	ENGINE_CORE_INFO("The gpu has a minimum buffer alignement of {0}", _gpuProperties.limits.minUniformBufferOffsetAlignment);
}

void VulkanRenderer::recreate_swapchain()
{	
	vkDeviceWaitIdle(_device);
	_swapDeletionQueue.flush();
	_swapChainObj.init_swapchain(_windowHandler->_window);

	init_framebuffers();	

	//update parts of the pipeline that change dynamically
	pipelineBuilder._viewport.width = (float)_swapChainObj._actualExtent.width;
	pipelineBuilder._viewport.height = (float)_swapChainObj._actualExtent.height;
	pipelineBuilder._scissor.extent = _swapChainObj._actualExtent;
}

void VulkanRenderer::init_default_renderpass()
{
	//we define an attachment description for our main color image
	//the attachment is loaded as "clear" when renderpass start
	//the attachment is stored when renderpass ends
	//the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
	//we dont care about stencil, and dont use multisampling

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = _swapChainObj._swapchainImageFormat;
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
	depth_attachment.format = _swapChainObj._depthFormat;
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
	
	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));
	_mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(_device, _renderPass, nullptr);
	});
}

void VulkanRenderer::init_framebuffers()
{
	const uint32_t swapchain_imagecount = _swapChainObj._swapchainImageViews.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	for (int i = 0; i < swapchain_imagecount; i++) {
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(_renderPass, _swapChainObj._actualExtent);
		std::array <VkImageView, 2> attachments = {_swapChainObj._swapchainImageViews[i], _swapChainObj._depthImageView};


		fb_info.attachmentCount = attachments.size();
		fb_info.pAttachments = attachments.data();
		VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));
		_swapDeletionQueue.push_function([=]() {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
		});
	}
}

void VulkanRenderer::init_commands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++) {

	
		VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
		_mainDeletionQueue.push_function([=]() {
			vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
		});
	}
	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));
	_mainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
	});
}

void VulkanRenderer::init_sync_structures()
{
	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) {

		VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

		//enqueue the destruction of the fence
		_mainDeletionQueue.push_function([=]() {
			vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
			});


		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

		//enqueue the destruction of semaphores
		_mainDeletionQueue.push_function([=]() {
			vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
			vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
			});
		
	}
	 VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

	VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
	});

}


void VulkanRenderer::init_pipelines()
{
	VkShaderModule colorMeshShader;
	//Program only compiles shaders if there's no .spv file available
	if (!vkcomponent::load_shader_module(vkcomponent::CompileGLSL(".Shaders/default_lit.frag").c_str(), &colorMeshShader, _device))
	{
		ENGINE_CORE_ERROR("Error when building the colored mesh shader");
	}
	VkShaderModule texturedMeshShader;
	if (!vkcomponent::load_shader_module(vkcomponent::CompileGLSL(".Shaders/textured_lit.frag").c_str(), &texturedMeshShader, _device))
	{
		ENGINE_CORE_ERROR("Error when building the textured mesh shader");
	}

	VkShaderModule meshVertShader;
	if (!vkcomponent::load_shader_module(vkcomponent::CompileGLSL(".Shaders/mesh_triangle.vert").c_str(), &meshVertShader, _device))
	{
		ENGINE_CORE_ERROR("Error when building the mesh vertex shader module");
	}

	
	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, colorMeshShader));


	//we start from just the default empty pipeline layout info
	VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

	//setup push constants
	VkPushConstantRange push_constant;
	//offset 0
	push_constant.offset = 0;
	//size of a MeshPushConstant struct
	push_constant.size = sizeof(MeshPushConstants);
	//for the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
	mesh_pipeline_layout_info.pushConstantRangeCount = 1;

	VkDescriptorSetLayout setLayouts[] = { _globalSetLayout, _objectSetLayout };

	mesh_pipeline_layout_info.setLayoutCount = 2;
	mesh_pipeline_layout_info.pSetLayouts = setLayouts;

	VkPipelineLayout meshPipLayout;
	VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &meshPipLayout));

	//we start from  the normal mesh layout
	VkPipelineLayoutCreateInfo textured_pipeline_layout_info = mesh_pipeline_layout_info;
		
	VkDescriptorSetLayout texturedSetLayouts[] = { _globalSetLayout, _objectSetLayout,_singleTextureSetLayout };

	textured_pipeline_layout_info.setLayoutCount = 3;
	textured_pipeline_layout_info.pSetLayouts = texturedSetLayouts;

	VkPipelineLayout texturedPipeLayout;
	VK_CHECK(vkCreatePipelineLayout(_device, &textured_pipeline_layout_info, nullptr, &texturedPipeLayout));

	//hook the push constants layout
	pipelineBuilder._pipelineLayout = meshPipLayout;

	//vertex input controls how to read vertices from vertex buffers. We arent using it yet
	pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)_swapChainObj._actualExtent.width;
	pipelineBuilder._viewport.height = (float)_swapChainObj._actualExtent.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = _swapChainObj._actualExtent;

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

	//we dont use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();


	//default depthtesting
	pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//build the mesh pipeline

	VertexInputDescription vertexDescription = Vertex::get_vertex_description();

	//connect the pipeline builder vertex input info to the one we get from Vertex
	pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
	pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

	pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
	pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

	
	//build the mesh triangle pipeline
	VkPipeline meshPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);

	create_material(meshPipeline, meshPipLayout, "defaultmesh");

	//create pipeline for textured drawing
	pipelineBuilder._shaderStages.clear();
	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

	pipelineBuilder._shaderStages.push_back(
		vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

	pipelineBuilder._pipelineLayout = texturedPipeLayout;
	VkPipeline texPipeline = pipelineBuilder.build_pipeline(_device, _renderPass);
	create_material(texPipeline, texturedPipeLayout, "texturedmesh");
	create_material(texPipeline, texturedPipeLayout, "texturedmesh2");

	vkDestroyShaderModule(_device, meshVertShader, nullptr);
	vkDestroyShaderModule(_device, colorMeshShader, nullptr);
	vkDestroyShaderModule(_device, texturedMeshShader, nullptr);

	_mainDeletionQueue.push_function([=]() {
		vkDestroyPipeline(_device, meshPipeline, nullptr);
		vkDestroyPipeline(_device, texPipeline, nullptr);
		vkDestroyPipelineLayout(_device, meshPipLayout, nullptr);
		vkDestroyPipelineLayout(_device, texturedPipeLayout, nullptr);
	});
}





void VulkanRenderer::load_meshes()
{
	Mesh cubeMesh{};
	//make the array 3 vertices long
	cubeMesh._vertices.resize(105);

	//vertex positions
	cubeMesh._vertices[0].position = {  -0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[1].position = {  0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[2].position = {  0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[3].position = {  0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[4].position = { -0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[5].position = {  -0.5f, -0.5f, -0.5f};

	cubeMesh._vertices[6].position = {-0.5f, -0.5f,  0.5f};  
	cubeMesh._vertices[7].position = {0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[8].position = {0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[9].position = {0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[10].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[11].position = {-0.5f, -0.5f,  0.5f};

	cubeMesh._vertices[12].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[13].position = {-0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[14].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[15].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[16].position = {-0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[17].position = {-0.5f,  0.5f,  0.5f};

	cubeMesh._vertices[18].position = {0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[19].position = {0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[20].position = {0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[21].position = {0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[22].position = {0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[23].position = {0.5f,  0.5f,  0.5f};

	cubeMesh._vertices[24].position = {-0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[25].position = {0.5f, -0.5f, -0.5f};
	cubeMesh._vertices[26].position = {0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[27].position = {0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[28].position = {-0.5f, -0.5f,  0.5f};
	cubeMesh._vertices[29].position = {-0.5f, -0.5f, -0.5f};

	cubeMesh._vertices[30].position = {-0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[31].position = {0.5f,  0.5f, -0.5f};
	cubeMesh._vertices[32].position = {0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[33].position = {0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[34].position = {-0.5f,  0.5f,  0.5f};
	cubeMesh._vertices[35].position = {-0.5f,  0.5f, -0.5f};
	


	//load the monkey
	Mesh monkeyMesh{};
	Mesh lostEmpire{};
	Mesh viking_room{};
	monkeyMesh.load_from_obj("EngineAssets/Meshes/monkey_smooth.obj");
	lostEmpire.load_from_obj("EngineAssets/Meshes/lost_empire.obj");
	viking_room.load_from_obj("EngineAssets/Meshes/viking_room.obj");

	//upload_mesh(triMesh);
	upload_mesh(monkeyMesh);
	upload_mesh(lostEmpire);
	upload_mesh(cubeMesh);
	upload_mesh(viking_room);

	_meshes["monkey"] = monkeyMesh;
	_meshes["skyBox"] = cubeMesh;	
	_meshes["empire"] = lostEmpire;
	_meshes["viking_room"] = viking_room;
}


void VulkanRenderer::upload_mesh(Mesh& mesh)
{
	const size_t bufferSize= mesh._vertices.size() * sizeof(Vertex);
	//allocate vertex buffer
	VkBufferCreateInfo stagingBufferInfo = {};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;
	//this is the total size, in bytes, of the buffer we are allocating
	stagingBufferInfo.size = bufferSize;
	//this buffer is going to be used as a Vertex Buffer
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	AllocatedBuffer stagingBuffer;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &stagingBufferInfo, &vmaallocInfo,
		&stagingBuffer._buffer,
		&stagingBuffer._allocation,
		nullptr));	

	//copy vertex data
	void* data;
	vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator, stagingBuffer._allocation);


	//allocate vertex buffer
	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.pNext = nullptr;
	//this is the total size, in bytes, of the buffer we are allocating
	vertexBufferInfo.size = bufferSize;
	//this buffer is going to be used as a Vertex Buffer
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	//let the VMA library know that this data should be gpu native	
	vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &vertexBufferInfo, &vmaallocInfo,
		&mesh._vertexBuffer._buffer,
		&mesh._vertexBuffer._allocation,
		nullptr));
	//add the destruction of triangle mesh buffer to the deletion queue
	_mainDeletionQueue.push_function([=]() {

		vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
		});

	immediate_submit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = bufferSize;
		vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh._vertexBuffer._buffer, 1, & copy);
	});

	vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);
}

void VulkanRenderer::load_images()
{
	Texture lostEmpire;
	Texture vikingRoom;
	asset_builder::convert_image("EngineAssets/Textures/lost_empire-RGBA.png", "EngineAssets/Textures/lost_empire-RGBA.bin");
	vkcomponent::load_image_from_asset(*this, "EngineAssets/Textures/lost_empire-RGBA.bin", lostEmpire.image);
	
	VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, lostEmpire.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device, &imageinfo, nullptr, &lostEmpire.imageView);

	_loadedTextures["empire_diffuse"] = lostEmpire;

	asset_builder::convert_image("EngineAssets/Textures/viking_room.png", "EngineAssets/Textures/viking_room.bin");
	vkcomponent::load_image_from_asset(*this, "EngineAssets/Textures/viking_room.bin", vikingRoom.image);
	
	VkImageViewCreateInfo imageinfo2 = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, vikingRoom.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device, &imageinfo2, nullptr, &vikingRoom.imageView);

	_loadedTextures["vikingroom_diffuse"] = vikingRoom;

	_mainDeletionQueue.push_function([=]() {
		vkDestroyImageView(_device, lostEmpire.imageView, nullptr);
		vkDestroyImageView(_device, vikingRoom.imageView, nullptr);
	});

}



Material* VulkanRenderer::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_materials[name] = mat;
	return &_materials[name];
}

Material* VulkanRenderer::get_material(const std::string& name)
{
	//search for the object, and return nullpointer if not found
	auto it = _materials.find(name);
	if (it == _materials.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}


Mesh* VulkanRenderer::get_mesh(const std::string& name)
{
	auto it = _meshes.find(name);
	if (it == _meshes.end()) {
		return nullptr;
	}
	else {
		return &(*it).second;
	}
}


void VulkanRenderer::draw_objects(VkCommandBuffer cmd,RenderObject* first, int count)
{

	GPUCameraData camData;
	camData.view = _camera.get_view_matrix();
	camData.proj = _camera.get_projection_matrix(false);
	camData.viewproj = camData.proj * camData.view ;
	camData.camPos = glm::vec4(glm::vec3(_camera.position), 0.0f);

	void* data;
	vmaMapMemory(_allocator, get_current_frame().cameraBuffer._allocation, &data);

	memcpy(data, &camData, sizeof(GPUCameraData));

	vmaUnmapMemory(_allocator, get_current_frame().cameraBuffer._allocation);

	_sceneParameters.lightData.lightPositions[0] = glm::vec4(glm::vec3(0.0f,  1.0f, 2.0f),0.0f);
	
	_sceneParameters.lightData.lightColors[0] = glm::vec4(glm::vec3(1.0f),0.0f);
	_sceneParameters.lightData.lightPositions[1] = glm::vec4(glm::vec3(0.0f,  -1.0f, -2.0f),0.0f);
	
	_sceneParameters.lightData.lightColors[1] = glm::vec4(glm::vec3(3.0f),0.0f);


	_sceneParameters.matData.albedo = glm::vec4(glm::vec3(1.0f,0.10f,0.0f),0.0f);
	_sceneParameters.matData.metallic = glm::vec4(glm::vec3(1.0f),0.0f);
	_sceneParameters.matData.roughness = glm::vec4(glm::vec3(0.25f),0.0f);
	_sceneParameters.matData.ao = glm::vec4(glm::vec3(1.0f),0.0f);

	


	
	char* sceneData;
	vmaMapMemory(_allocator, _sceneParameterBuffer._allocation , (void**)&sceneData);

	int frameIndex = _frameNumber % FRAME_OVERLAP;

	sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

	memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));

	vmaUnmapMemory(_allocator, _sceneParameterBuffer._allocation);


	void* objectData;
	vmaMapMemory(_allocator, get_current_frame().objectBuffer._allocation, &objectData);
	
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		objectSSBO[i].modelMatrix = object.transformMatrix;
	}
	
	vmaUnmapMemory(_allocator, get_current_frame().objectBuffer._allocation);

	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;
	
	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];
		//only bind the pipeline if it doesnt match with the already bound one
		if (object.material != lastMaterial) {

			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
			vkCmdSetViewport(cmd, 0, 1, &pipelineBuilder._viewport);
			vkCmdSetScissor(cmd, 0, 1, &pipelineBuilder._scissor);
			lastMaterial = object.material;

			uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0, 1, &get_current_frame().globalDescriptor, 1, &uniform_offset);

			//object data descriptor
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1, 1, &get_current_frame().objectDescriptor, 0, nullptr);
			if (object.material->textureSet != VK_NULL_HANDLE) {
				//texture descriptor
				temptextureSet = object.material->textureSet;
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 2, 1, &temptextureSet, 0, nullptr);
			}
		}

		glm::mat4 model = object.transformMatrix;
		//final render matrix, that we are calculating on the cpu
		glm::mat4 mesh_matrix = model;

		MeshPushConstants constants;
		constants.render_matrix = mesh_matrix;

		//upload the mesh to the gpu via pushconstants
		vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
		//only bind the mesh if its a different one from last bind
		if (object.mesh != lastMesh) {
			//bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
			lastMesh = object.mesh;
		}
		//we can now draw
		vkCmdDraw(cmd, object.mesh->_vertices.size(), 1,0 , i);
	}
}



void VulkanRenderer::init_scene()
{
	RenderObject monkey;
	monkey.mesh = get_mesh("monkey");
	monkey.material = get_material("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };

	_renderables.push_back(monkey);

	RenderObject map;
	map.mesh = get_mesh("empire");
	map.material = get_material("texturedmesh");
	map.transformMatrix = glm::translate(glm::vec3{ 5,-12,0 }); 

	_renderables.push_back(map);

	RenderObject viking_room;
	viking_room.mesh = get_mesh("viking_room");
	viking_room.material = get_material("texturedmesh2");
	viking_room.transformMatrix = glm::translate(glm::vec3{ 0,0,2.0f }); 

	_renderables.push_back(viking_room);
	
	RenderObject skyBox;
	skyBox.mesh = get_mesh("skyBox");
	skyBox.material = get_material("defaultmesh");
	glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(0, 2, 0));
	glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(1.0f, 1.0f, 1.0f));
	skyBox.transformMatrix = translation * scale;
	_renderables.push_back(skyBox);
	
	//create a sampler for the texture
	VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VkSampler blockySampler;
	vkCreateSampler(_device, &samplerInfo, nullptr, &blockySampler);

	//create a sampler for the texture
	VkSamplerCreateInfo samplerInfo2 = vkinit::sampler_create_info(VK_FILTER_NEAREST);

	VkSampler blockySampler2;
	vkCreateSampler(_device, &samplerInfo2, nullptr, &blockySampler2);

	Material* texturedMat =	get_material("texturedmesh");
	_descriptorAllocator->allocate(&texturedMat->textureSet, _singleTextureSetLayout);

	Material* texturedMat2 = get_material("texturedmesh2");
	_descriptorAllocator->allocate(&texturedMat2->textureSet, _singleTextureSetLayout);

	//write to the descriptor set so that it points to our empire_diffuse texture
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = blockySampler;
	imageBufferInfo.imageView = _loadedTextures["empire_diffuse"].imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo imageBufferInfo2;
	imageBufferInfo2.sampler = blockySampler2;
	imageBufferInfo2.imageView = _loadedTextures["vikingroom_diffuse"].imageView;
	imageBufferInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat->textureSet, &imageBufferInfo, 0);
	VkWriteDescriptorSet texture2 = vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texturedMat2->textureSet, &imageBufferInfo2, 0);
	vkUpdateDescriptorSets(_device, 1, &texture1, 0, nullptr);	
	vkUpdateDescriptorSets(_device, 1, &texture2, 0, nullptr);	

	_mainDeletionQueue.push_function([=]() {
		vkDestroySampler(_device, blockySampler, nullptr);
		vkDestroySampler(_device, blockySampler2, nullptr);
	});
}

AllocatedBuffer VulkanRenderer::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;


	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;

	AllocatedBuffer newBuffer;

	//allocate the buffer
	VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
		&newBuffer._buffer,
		&newBuffer._allocation,
		nullptr));

	return newBuffer;
}

size_t VulkanRenderer::pad_uniform_buffer_size(size_t originalSize)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0) {
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}


void VulkanRenderer::init_descriptors()
{
	_descriptorAllocator = new vkcomponent::DescriptorAllocator{};
	_descriptorAllocator->init(_device);

	_descriptorLayoutCache = new vkcomponent::DescriptorLayoutCache{};
	_descriptorLayoutCache->init(_device);
	
	VkDescriptorSetLayoutBinding cameraBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0);
	VkDescriptorSetLayoutBinding sceneBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	
	VkDescriptorSetLayoutBinding bindings[] = { cameraBind,sceneBind };

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.bindingCount = 2;
	setinfo.flags = 0;
	setinfo.pNext = nullptr;
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pBindings = bindings;

	_globalSetLayout = _descriptorLayoutCache->create_descriptor_layout(&setinfo);

	VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set2info = {};
	set2info.bindingCount = 1;
	set2info.flags = 0;
	set2info.pNext = nullptr;
	set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set2info.pBindings = &objectBind;

	_objectSetLayout = _descriptorLayoutCache->create_descriptor_layout(&set2info);

	VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

	VkDescriptorSetLayoutCreateInfo set3info = {};
	set3info.bindingCount = 1;
	set3info.flags = 0;
	set3info.pNext = nullptr;
	set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set3info.pBindings = &textureBind;

	_singleTextureSetLayout = _descriptorLayoutCache->create_descriptor_layout(&set3info);

	const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
	_sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_frames[i].dynamicDescriptorAllocator = new vkcomponent::DescriptorAllocator{};
		_frames[i].dynamicDescriptorAllocator->init(_device);
		_frames[i].cameraBuffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		const int MAX_OBJECTS = 10000;
		_frames[i].objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_frames[i].dynamicDescriptorAllocator->allocate(&_frames[i].globalDescriptor, _globalSetLayout);

		_frames[i].dynamicDescriptorAllocator->allocate(&_frames[i].objectDescriptor, _objectSetLayout);

		VkDescriptorBufferInfo cameraInfo;
		cameraInfo.buffer = _frames[i].cameraBuffer._buffer;
		cameraInfo.offset = 0;
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo;
		sceneInfo.buffer = _sceneParameterBuffer._buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo;
		objectBufferInfo.buffer = _frames[i].objectBuffer._buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;


		VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frames[i].globalDescriptor,&cameraInfo,0);
		
		VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frames[i].globalDescriptor, &sceneInfo, 1);

		VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i].objectDescriptor, &objectBufferInfo, 0);

		VkWriteDescriptorSet setWrites[] = { cameraWrite,sceneWrite,objectWrite };

		vkUpdateDescriptorSets(_device, 3, setWrites, 0, nullptr);
		_mainDeletionQueue.push_function([=]()
		{
			vmaDestroyBuffer(_allocator, _frames[i].objectBuffer._buffer, _frames[i].objectBuffer._allocation);
			vmaDestroyBuffer(_allocator, _frames[i].cameraBuffer._buffer, _frames[i].cameraBuffer._allocation);
		});
	}
	_mainDeletionQueue.push_function([=]() {
		vmaDestroyBuffer(_allocator, _sceneParameterBuffer._buffer, _sceneParameterBuffer._allocation);
		for (auto& frame : _frames)
		{
			frame.dynamicDescriptorAllocator->cleanup();
		}
		_descriptorAllocator->cleanup();
		_descriptorLayoutCache->cleanup();
	});
	
}


void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

    VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &cmd));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::submit_info(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

	vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
	vkResetFences(_device, 1, &_uploadContext._uploadFence);

    //clear the command pool. This will free the command buffer too
	vkResetCommandPool(_device, _uploadContext._commandPool, 0);
}
