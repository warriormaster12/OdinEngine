#pragma once 

#include "vk_types.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class VulkanRenderer;

struct LightMatrixData{   
	glm::mat4 lightSpaceMatrix;
};

// here we defie things to get offscreen rendering going
class VulkanOffscreen
{
public:
    void InitOffscreen(VulkanRenderer& renderer);
    void InitFramebuffer();
    void BeginOffscreenRenderpass();
    void EndOffscreenRenderpass();
private:
    VulkanRenderer* p_renderer;
    void InitRenderpass();
    void InitDescriptors();
    void InitPipelines();

    //shadows
	Texture shadowImage;
	VkFramebuffer shadowFramebuffer;
	VkExtent2D shadowExtent{1024, 1024};

    VkRenderPass shadowPass;

    VkPipeline shadowDebug;
    VkPipelineLayout shadowDebugLayout;

    VkSampler shadowMapSampler;

    VkDescriptorSetLayout offscreenGlobalSetLayout{};
	VkDescriptorSetLayout offscreenObjectSetLayout{};

};