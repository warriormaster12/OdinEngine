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

	glm::vec4 emissionColor; //vec3
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

struct DirectionLight{
    glm::vec4 direction; //vec3
    glm::vec4 color; //vec3
	glm::vec4 intensity; //float
};

struct PointLight
{
	glm::vec4 position; // vec3
	glm::vec4 color; // vec3
    glm::vec4 radius; //float
	glm::vec4 intensity; //float
};

struct GPUSceneData {
	glm::vec4 plightCount; //int
	DirectionLight dLight;
	PointLight pointLights[3];
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
	/**
	 * Initializes the Vulkan renderer.
	 * 
	 * @param windowHandler The window handler object
	 */
	void Init(WindowHandler& windowHandler);

	/**
	 * Destroys and cleans up the Vulkan renderer.
	 */
	void CleanUp();

	/**
	 * Starts a new frame draw. 
	 * 
	 * Must be called at the start of the frame and before DrawObjects() and EndDraw().
	 */
	void BeginDraw();

	/**
	 * Ends the current frame draw. 
	 * 
	 * Must be called at the end of the frame and before the next BeginDraw().
	 */
	void EndDraw();

	/**
	 * Draws a set of objects. 
	 * 
	 * @param objects A vector of objects to draw
	 */
	void DrawObjects(const std::vector<RenderObject>& objects);
	
	/**
	 * Executes a function and submits the command buffer immediately.
	 * 
	 * @param function The function that is used to add and modify the command buffer
	 */
	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	/**
	 * Adds a function to be called during cleanup. 
	 * 
	 * The added functions are called in FIFO order.
	 * 
	 * @param function The function to call during cleanup
	 */
	void EnqueueCleanup(std::function<void()>&& function);

	/*********************************************************************/
	/* Things that will be moved out of the renderer in the near future. */
	/*********************************************************************/

	/**
	 * Loads a mesh. 
	 * 
	 * Initializes the vertex and, in the future, index buffer for the mesh. It also enqueues 
	 * the deletion of those buffers when CleanUp() is called.
	 * 
	 * @param mesh The mesh to load
	 */
	void UploadMesh(Mesh& mesh);

	/*
	 * Creates a default material and stores it by name. 
	 * 
	 * This will create the necessary bindings and store the material by the name. Note that 
	 * the material stores the pipeline (for now). This means that you will need to create a 
	 * duplicate material for each pipeline. 
	 * 
	 * In the future, pipelines will move into their own storage and this function will 
	 * take in material parameters.
	 * 
	 * @param pipeline The pipeline to use this material in
	 * @param layout The layout of the pipeline
	 * @param name The name to store it as
	 */
	void CreateMaterial(VkPipeline& pipeline, VkPipelineLayout& layout, const std::string& name);

	/**
	 * Gets a material by its name. 
	 * 
	 * @param name The name of the material
	 * @return A pointer to the material or nullptr if it could not be found
	 */
	Material* GetMaterial(const std::string& name);

	/**
	 * Loads a texture from the disk and binds it to a material.
	 * 
	 * @param materialName The name of the material to bind the texture to
	 * @param texturePath The path to the texture file
	 * @param index The material texture index.
	 * index=0: Albedo texture
	 * index=1: Ambient Occlusion map
	 * index=2: Normal map
	 * index=3: Emission map
	 * index=4: Metallic roughness map
	 * index=5: Metallic map
	 * index=6: Roughness map
	 */
	void CreateTexture(std::string materialName, const std::string texturePath, uint32_t index);

	Camera& GetCamera() { return camera; }
	const VmaAllocator& GetAllocator() const { return allocator; }

	VkInstance& GetInstance() { return instance; }
	VkPhysicalDevice& GetPhysicalDevice() { return chosenGPU; }
	VkDevice& GetDevice() { return device; }
	VkQueue& GetGraphicsQueue() { return graphicsQueue; }
	VkRenderPass& GetRenderPass() { return renderPass; }

	uint32_t GetWidth() const { return swapChainObj.actualExtent.width; }
	uint32_t GetHeight() const { return swapChainObj.actualExtent.height; }
	
private:
	int frameNumber{0};
	int selectedShader{0};
	WindowHandler* p_windowHandler;

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

	VkSampler textureSampler;

	GPUSceneData sceneParameters;
	AllocatedBuffer sceneParameterBuffer;

	vkcomponent::SwapChain swapChainObj{chosenGPU, device, allocator, swapDeletionQueue};
	Camera camera{swapChainObj};

	UploadContext uploadContext;

	std::unordered_map<std::string, Texture> loadedTextures;
	std::unordered_map<std::string, Material> materials;
	std::vector<std::string> materialList; 

	void InitVulkan();
	void InitSwapchain();
	void InitDefaultRenderpass();
	void InitFramebuffers();
	void InitCommands();
	void InitSyncStructures();
	void InitDescriptors();
	void InitSamplers();
	void InitPipelines();
	void RecreateSwapchain();

	FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }
	FrameData& GetLastFrame() { return frames[(frameNumber - 1) % FRAME_OVERLAP]; }

	void LoadImage(const std::string textureName, const std::string texturePath);
	//this is done when creating new material
	void AllocateEmptyTextures(const std::string& materialName, VkSampler& sampler);
};

