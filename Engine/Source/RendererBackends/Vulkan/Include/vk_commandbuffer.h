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
    static void BeginCommands(VkCommandBuffer& cmd, uint32_t& imageIndex, FunctionQueuer recreateSwapchain);
    static void EndCommands(FunctionQueuer recreateSwapchain);

    inline static FrameData frames[FRAME_OVERLAP];
    static FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }
    
private:
    inline static UploadContext uploadContext;

    
};