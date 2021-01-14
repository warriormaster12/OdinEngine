#pragma once

#include "vk_types.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "../vk_components/Include/vk_swapchain.h"
#include "../vk_components/Include/vk_deletionqueue.h"
#include "../vk_components/Include/vk_descriptors.h"
#include "../vk_components/Include/vk_mesh.h"

#include "../../Window/Include/WindowHandler.h"

#include "../../Camera/Include/camera.h"



struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};


struct Material {
	VkDescriptorSet textureSet{VK_NULL_HANDLE};
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;

	Material* material;

	glm::mat4 transformMatrix;
};


struct FrameData {
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	vkcomponent::DeletionQueue _frameDeletionQueue;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	vkcomponent::DescriptorAllocator* dynamicDescriptorAllocator;

	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;
};

struct GPUCameraData{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};


struct GPUSceneData {
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; //x for min, y for max, zw unused.
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; //w for sun power
	glm::vec4 sunlightColor;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct UploadContext {
	VkFence _uploadFence;
	VkCommandPool _commandPool;	
};

struct Texture {
	AllocatedImage image;
	VkImageView imageView;
};


constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer {
public:
	int _frameNumber {0};
	int _selectedShader{ 0 };


	VkDescriptorSet temptextureSet{VK_NULL_HANDLE};
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkPhysicalDevice _chosenGPU;
	VkDevice _device;

	VkPhysicalDeviceProperties _gpuProperties;

	FrameData _frames[FRAME_OVERLAP];
	
	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;
	
	VkRenderPass _renderPass;

	std::vector<VkFramebuffer> _framebuffers;
	

    vkcomponent::DeletionQueue _mainDeletionQueue;
	vkcomponent::DeletionQueue _swapDeletionQueue;
	
	VmaAllocator _allocator; //vma lib allocator

	vkcomponent::DescriptorAllocator* _descriptorAllocator;
	vkcomponent::DescriptorLayoutCache* _descriptorLayoutCache;


	VkDescriptorSetLayout _globalSetLayout{};
	VkDescriptorSetLayout _objectSetLayout{};
	VkDescriptorSetLayout _singleTextureSetLayout{};

	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParameterBuffer;

	vkcomponent::SwapChain _swapChainObj{_chosenGPU, _device, _allocator, _swapDeletionQueue};
	Camera _camera{_swapChainObj};

	//texture hashmap
	std::unordered_map<std::string, Texture> _loadedTextures;
	void load_images();
	//initializes everything in the engine
	void init(WindowHandler& windowHandler);

	//shuts down the engine
	void cleanup();

	//draw loop
	void draw();

	//run main loop
	void run();
	
	FrameData& get_current_frame();
	FrameData& get_last_frame();

	//default array of renderable objects
	std::vector<RenderObject> _renderables;

	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, Mesh> _meshes;

	UploadContext _uploadContext;

	//functions
	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

	//create material and add it to the map
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it cant be found
	Material* get_material(const std::string& name);

	//returns nullptr if it cant be found
	Mesh* get_mesh(const std::string& name);

	//our draw function
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);

	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	size_t pad_uniform_buffer_size(size_t originalSize);

	WindowHandler* _windowHandler;

	bool frameBufferResized = false;

	void frameBufferResize();
	
private:

	void init_vulkan();

	void init_default_renderpass();

	void init_framebuffers();

	void init_commands();

	void init_sync_structures();

	void init_pipelines();

	void init_scene();

	void init_descriptors();

	void load_meshes();

	void upload_mesh(Mesh& mesh);

	void recreate_swapchain();
};




