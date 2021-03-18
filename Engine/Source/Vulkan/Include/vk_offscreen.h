#pragma once 

#include "vk_types.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "mesh_core.h"
#include "vk_descriptors.h"
#include "camera.h"

#define SHADOW_MAP_CASCADE_COUNT 4
class VulkanRenderer;


struct PushConstBlock {
    glm::vec4 position;
    uint32_t cascadeIndex;
};

struct DepthPass {
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    struct UniformBlock {
        std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> cascadeViewProjMat;
    } ubo;

    AllocatedBuffer uboBuffer;

} ;
struct DepthImage {
    Texture depthImage;
    VkSampler sampler;
    VkExtent2D imageSize {4096, 4096};
};
struct Cascade {
		VkFramebuffer frameBuffer;
		VkDescriptorSet descriptorSet;
		VkImageView view;

		float splitDepth;
		glm::mat4 viewProjMatrix;

    void destroy(VkDevice device) {
        vkDestroyImageView(device, view, nullptr);
        vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
};


// here we defie things to get offscreen rendering going
class VulkanOffscreen
{
public:
    void InitOffscreen(VulkanRenderer& renderer);
    void InitFramebuffer();
    void BeginOffscreenRenderpass(const uint32_t& count);
    void updateLight(float dt);
    void calculateCascades(Camera& camera);
    void drawOffscreenShadows(const std::vector<RenderObject>& objects, uint32_t count);
    void debugShadows(bool debug = false);
    void EndOffscreenRenderpass();

    Cascade& GetCurrenCascade(uint32_t cascadeIndex = 0) { return cascades[cascadeIndex];}
    DepthImage& GetDepthImage() { return depth;}
    glm::vec3 GetLightDir() {return lightDir;}

    
private:
    VulkanRenderer* p_renderer;
    void InitRenderpass();
    void InitDescriptors();
    void InitPipelines();
    void BuildImage();

    //Shadow shadow;
    DepthImage depth;
    DepthPass depthPass;

    VkPipeline shadowDebug;
    VkPipelineLayout shadowDebugLayout;

    std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;

    VkDescriptorSetLayout debugSetLayout{};
    VkDescriptorSetLayout depthSetLayout{};
    VkDescriptorSet depthSet;
    glm::vec3 lightDir;
	

};