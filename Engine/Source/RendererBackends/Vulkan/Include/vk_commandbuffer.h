#pragma once 

#include "vk_types.h"
#include "function_queuer.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>

constexpr unsigned int FRAME_OVERLAP = 2;
static int frameNumber;

struct FrameData {
	VkSemaphore presentSemaphore, renderSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

    std::unordered_map<std::string, VkDescriptorSet> descriptorSets;
};
struct UploadContext {
	VkFence uploadFence;
	VkCommandPool commandPool;	
};

class VkCommandbufferManager
{
public:
    static void InitCommands();
    static void InitSyncStructures();
    static void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
    static void CleanUpCommands();

    //these functions are used to start drawing loop
    static void BeginCommands(std::function<void()> recreateSwapchain);
    static void EndCommands(std::function<void()> recreateSwapchain);

    inline static FrameData frames[FRAME_OVERLAP];
    static FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

    static uint32_t& GetImageIndex(){return imageIndex;}
    static VkCommandBuffer& GetCommandBuffer() {return cmd;}
    
private:
    inline static UploadContext uploadContext;
    inline static VkCommandBuffer cmd;
    inline static uint32_t imageIndex;
};