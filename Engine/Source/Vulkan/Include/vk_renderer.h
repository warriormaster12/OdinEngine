#pragma once

#include "vk_types.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "vk_swapchain.h"
#include "vk_deletionqueue.h"
#include "vk_descriptors.h"
#include "vk_mesh.h"

#include "window_handler.h"

#include "camera.h"



struct Material {
	VkDescriptorSet materialSet{VK_NULL_HANDLE};
	AllocatedBuffer buffer;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	glm::vec4 albedo; // vec4
	float metallic; // float
	float roughness; // float
	float ao; // float

	glm::vec3 emissionColor; //vec3
    float emissionPower; // float

	
};

struct RenderObject {
	Mesh* p_mesh;

	Material* p_material;

	glm::mat4 transformMatrix;
};

struct DescriptorSetData {
	/* Descriptor set for uniform data */
	VkDescriptorSet uniform;
	/* Dynamic offset for 'uniform' descriptor set */
	uint32_t uniformOffset;
	/* Descriptor set for object data */
	VkDescriptorSet object;
	/* Dynamic offset for 'object' descriptor set */
	uint32_t objectOffset;
};

struct DrawCall {
	Mesh* pMesh;
	Material* pMaterial;
	/* Descriptor sets to bind during draw call*/
	DescriptorSetData descriptorSets;
	/* Transform matrix of the object */
	glm::mat4 transformMatrix;
	/* Start index of the first object to draw */
	uint32_t index;
	/* Number of objects to draw, starting from 'index' */
	uint32_t count;
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

	AllocatedBuffer indirectDrawBuffer;
	AllocatedBuffer objectBuffer;
	VkDescriptorSet objectDescriptor;
};

struct GPUCameraData{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;

	glm::vec4 camPos; // vec3
};

struct GPUMaterialData 
{
	glm::vec4 albedo; // vec4
	glm::vec4 metallic; // float
	glm::vec4 roughness; // float
	glm::vec4 ao; // float

	glm::vec4 emissionColor; //vec3
    glm::vec4 emissionPower; // float
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
	VkDescriptorSetLayout materialTextureSetLayout{};

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

	void UploadMesh(Mesh& mesh);
	
	FrameData& GetCurrentFrame();
	FrameData& GetLastFrame();

	//default array of renderable objects

	std::unordered_map<std::string, Material> materials;
	std::vector<std::string> materialList; 

	UploadContext uploadContext;

	//functions
	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	//create material and add it to the map
	Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	//returns nullptr if it cant be found
	Material* GetMaterial(const std::string& name);

	

	//our draw function
	void DrawObjects(const std::vector<RenderObject>& p_objects);

	size_t PadUniformBufferSize(size_t originalSize);

	WindowHandler* p_windowHandler;

	
private:

	void InitVulkan();

	void InitDefaultRenderpass();

	void InitFramebuffers();

	void InitCommands();

	void InitSyncStructures();

	void InitPipelines();

	void InitScene();

	void InitDescriptors();

	void RecreateSwapchain();

	void CreateTexture(std::string materialName, std::string textureName, VkSampler& sampler, uint32_t binding = 1);

};




