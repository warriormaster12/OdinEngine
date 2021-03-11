#pragma once 

#include "vk_types.h"

class VulkanRenderer;

// here we defie things to get offscreen rendering going
class VulkanOffscreen
{
public:
    void InitOffscreen(VulkanRenderer& renderer);

    void BeginOffscreenRenderpass();
    void EndOffscreenRenderpass();
private:
    VulkanRenderer* p_renderer;
    void InitRenderpass();
    void InitFramebuffer();
    void InitDescriptors();
    void InitPipelines();

    //shadows
	Texture shadowImage;
	VkFramebuffer shadowFramebuffer;
	VkExtent2D shadowExtent{1024, 1024};

    VkRenderPass shadowPass;

};