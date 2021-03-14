#pragma once 

#include "vk_types.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "mesh_core.h"

#define SHADOW_MAP_CASCADE_COUNT 4
class VulkanRenderer;


struct PushConstBlock {
    glm::vec4 position;
    uint32_t cascadeIndex;
};

struct Shadow{
    //shadows
	Texture shadowImage;
	VkFramebuffer shadowFramebuffer;
	VkExtent2D shadowExtent{4096, 4096};

    VkRenderPass shadowPass;
    VkPipeline shadowPipeline;
    VkPipelineLayout shadowPipelineLayout;


    struct LightMatrixData {
        std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> cascadeViewProjMat;
    } lightMatrixData;

    VkSampler shadowMapSampler;

    // Constant depth bias factor (always applied)
	float depthBiasConstant = 1.25f;
	// Slope depth bias factor, applied depending on polygon's slope
	float depthBiasSlope = 1.75f;
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
    void BeginOffscreenRenderpass();
    void drawOffscreenShadows(const std::vector<RenderObject>& objects);
    void debugShadows(bool debug = false);
    void EndOffscreenRenderpass();

    Shadow& GetShadow(){return shadow;}
    Shadow::LightMatrixData light;
private:
    VulkanRenderer* p_renderer;
    void InitRenderpass();
    void InitDescriptors();
    void InitPipelines();
    void BuildImage();

    Shadow shadow;
    std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;

    VkPipeline shadowDebug;
    VkPipelineLayout shadowDebugLayout;

    VkDescriptorSetLayout debugSetLayout{};
    VkDescriptorSet debugTextureSet;

    VkDescriptorSetLayout depthSetLayout{};

    VkDescriptorSetLayout depthSetLayoutGlobal{};
    VkDescriptorSet depthTextureSet;
	

};