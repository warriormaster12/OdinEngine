#pragma once

#include "vk_types.h"
#include "vk_mesh.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

#include "../vk_components/Include/vk_swapchain.h"
#include "../vk_components/Include/vk_deletionqueue.h"
#include "../vk_components/Include/vk_check.h"

struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};




struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;

	Material* material;

	glm::mat4 transformMatrix;
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

	VkPipelineLayout _meshPipelineLayout;
	VkPipeline  _meshPipeline;

	Mesh _triangleMesh;
	Mesh _monkeyMesh;

	bool _isInitialized{ false };
	int _frameNumber {0};

	vkcomponent::DeletionQueue _mainDeletionQueue;

	int _selectedShader{ 0 };

    struct SDL_Window* _window{ nullptr };

	//default array of renderable objects
	std::vector<RenderObject> _renderables;

	std::unordered_map<std::string,Material> _materials;
	std::unordered_map<std::string,Mesh> _meshes;
	//functions

	//create material and add it to the map
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout,const std::string& name);

	//returns nullptr if it can't be found
	Material* get_material(const std::string& name);

	//returns nullptr if it can't be found
	Mesh* get_mesh(const std::string& name);

	//our draw function
	void draw_objects(VkCommandBuffer cmd,RenderObject* first, int count);


	
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

	void init_scene();
	
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

	VkPipelineDepthStencilStateCreateInfo _depthStencil;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};


