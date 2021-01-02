#pragma once

#include "vk_types.h"
#include "vk_mesh.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../vk_components/Include/vk_swapchain.h"
#include "../vk_components/Include/vk_deletionqueue.h"

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};







class VulkanEngine {
public:
    // --- ommited --- 
    VkInstance _instance; // Vulkan library handle
	VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice _chosenGPU; // GPU chosen as the default device
	VkDevice _device; // Vulkan device for commands

	vkcomponent::SwapChain _swapChain;
	VkQueue _graphicsQueue; //queue we will submit to
	uint32_t _graphicsQueueFamily; //family of that queue

	VkCommandPool _commandPool; //the command pool for our commands
	VkCommandBuffer _mainCommandBuffer; //the buffer we will record into

	VkRenderPass _renderPass;

	VmaAllocator _allocator; //vma lib allocator

	std::vector<VkFramebuffer> _framebuffers;

	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;


	bool _isInitialized{ false };
	int _frameNumber {0};

	vkcomponent::DeletionQueue _mainDeletionQueue;

	int _selectedShader{ 0 };

    struct SDL_Window* _window{ nullptr };

	VkPipelineLayout _meshPipelineLayout;
	VkPipeline _meshPipeline;
	Mesh _triangleMesh;
	
	//initializes everything in the engine
	void init();

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();


	//loads a shader module from a spir-v file. Returns false if it errors
	bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

private:
    void init_vulkan();	
	void init_commands();
	
	void init_default_renderpass();
	void init_framebuffers();

	void init_pipelines();

	void init_sync_structures();

	void load_meshes();

	void upload_mesh(Mesh& mesh);
	
};


class PipelineBuilder {
public:

	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};


