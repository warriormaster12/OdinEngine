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

#include "../../Window/Include/window_handler.h"

#include "../../Camera/Include/camera.h"



struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 render_matrix;
};


struct Material {
	VkDescriptorSet textureSet{VK_NULL_HANDLE};
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	glm::vec4 albedo; // vec4
	float metallic; // float
	float roughness; // float
	float ao; // float
};

struct RenderObject {
	Mesh* p_mesh;

	Material* p_material;

	glm::mat4 transformMatrix;
};


struct FrameData {
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	vkcomponent::DeletionQueue frameDeletionQueue;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	AllocatedBuffer cameraBuffer;
	VkDescriptorSet globalDescriptor;

	vkcomponent::DescriptorAllocator* p_dynamicDescriptorAllocator;

	AllocatedBuffer objectBuffer;
	AllocatedBuffer objectFragBuffer;
	VkDescriptorSet objectDescriptor;
};

struct GPUCameraData{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;

	glm::vec4 camPos; // vec3
};

struct MaterialData 
{
	glm::vec4 albedo; // vec4
	glm::vec4 metallic; // float
	glm::vec4 roughness; // float
	glm::vec4 ao; // float
};

struct Light
{
	glm::vec4 lightPositions[2]; // vec3
	glm::vec4 lightColors[2]; // vec3
};

struct GPUSceneData {
	Light lightData;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct GPUObjectFragData {
	MaterialData matData;
};

struct UploadContext {
	VkFence uploadFence;
	VkCommandPool commandPool;	
};

struct Texture {
	AllocatedImage image;
	VkImageView imageView;
};


constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer {
public:
	int frameNumber {0};
	int selectedShader{ 0 };


	VkDescriptorSet tempTextureSet{VK_NULL_HANDLE};
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice chosenGPU;
	VkDevice device;

	VkPhysicalDeviceProperties gpuProperties;

	FrameData frames[FRAME_OVERLAP];
	
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
	
	VkRenderPass renderPass;

	std::vector<VkFramebuffer> framebuffers;
	

    vkcomponent::DeletionQueue mainDeletionQueue;
	vkcomponent::DeletionQueue swapDeletionQueue;
	
	VmaAllocator allocator; //vma lib allocator

	vkcomponent::DescriptorAllocator* p_descriptorAllocator;
	vkcomponent::DescriptorLayoutCache* p_descriptorLayoutCache;


	VkDescriptorSetLayout globalSetLayout{};
	VkDescriptorSetLayout objectSetLayout{};
	VkDescriptorSetLayout singleTextureSetLayout{};

	GPUSceneData sceneParameters;
	AllocatedBuffer sceneParameterBuffer;

	vkcomponent::SwapChain swapChainObj{chosenGPU, device, allocator, swapDeletionQueue};
	Camera camera{swapChainObj};

	//texture hashmap
	std::unordered_map<std::string, Texture> _loadedTextures;
	void LoadImage(std::string texture_name, std::string texture_path);
	//initializes everything in the engine
	void Init(WindowHandler& windowHandler);

	//shuts down the engine
	void CleanUp();


	//run main loop
	void BeginDraw();
	void EndDraw();
	
	FrameData& GetCurrentFrame();
	FrameData& GetLastFrame();

	//default array of renderable objects
	std::vector<RenderObject> renderables;

	std::unordered_map<std::string, Material> materials;
	std::vector<std::string> materialList; 
	std::unordered_map<std::string, Mesh> meshes;

	UploadContext uploadContext;

	//functions
	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	//create material and add it to the map
	Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it cant be found
	Material* GetMaterial(const std::string& name);

	//returns nullptr if it cant be found
	Mesh* GetMesh(const std::string& name);

	//our draw function
	void DrawObjects(RenderObject* p_objects, int count);

	AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	size_t PadUniformBufferSize(size_t originalSize);

	WindowHandler* p_windowHandler;

	bool frameBufferResized = false;

	void FrameBufferResize();
	
private:

	void InitVulkan();

	void InitDefaultRenderpass();

	void InitFramebuffers();

	void InitCommands();

	void InitSyncStructures();

	void InitPipelines();

	void InitScene();

	void InitDescriptors();

	void LoadMeshes();

	void UploadMesh(Mesh& mesh);

	void RecreateSwapchain();

	void CreateTexture(std::string materialName, std::string textureName, VkSampler& sampler, uint32_t binding = 0);
};




