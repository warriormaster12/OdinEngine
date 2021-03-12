#pragma once 

#include "vk_types.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "mesh_core.h"

class VulkanRenderer;

struct LightMatrixData{   
	glm::mat4 lightSpaceMatrix;
};
struct Shadow{
    //shadows
	Texture shadowImage;
	VkFramebuffer shadowFramebuffer;
	VkExtent2D shadowExtent{1024, 1024};

    VkRenderPass shadowPass;
    VkPipeline shadowPipeline;
    VkPipelineLayout shadowPipelineLayout;

    VkDescriptorSetLayout offscreenGlobalSetLayout{};
	VkDescriptorSetLayout offscreenObjectSetLayout{};

    VkSampler shadowMapSampler;

    // Constant depth bias factor (always applied)
	float depthBiasConstant = 1.25f;
	// Slope depth bias factor, applied depending on polygon's slope
	float depthBiasSlope = 1.75f;
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
private:
    VulkanRenderer* p_renderer;
    void InitRenderpass();
    void InitDescriptors();
    void InitPipelines();
    void BuildImage();

    Shadow shadow;

    VkPipeline shadowDebug;
    VkPipelineLayout shadowDebugLayout;

    VkDescriptorSetLayout debugTextureLayout{};
    VkDescriptorSet debugTextureSet;

};