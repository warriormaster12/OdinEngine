#include "Include/vk_context.h"
#include "Include/vk_device.h"
#include "Include/vk_swapchain.h"
#include "Include/vk_commandbuffer.h"
#include "Include/vk_init.h"

#include "vk_check.h"
#include "vk_utils.h"
#include "window_handler.h"

#include "logger.h"
#include <iostream>
#include <unordered_map>
#include <vulkan/vulkan_core.h>




std::unordered_map<std::string, VkRenderPass> renderPass;
std::vector<std::unordered_map<std::string, VkFramebuffer>>frameBuffers;

//objects deleted on application closed
FunctionQueuer mainDeletionQueue;
//objects deleted on application window resize
FunctionQueuer swapDeletionQueue;

std::unordered_map<std::string, ShaderProgram> shaderProgram;
std::unordered_map<std::string, DescriptorSetInfo> descriptorSets;

vkcomponent::DescriptorAllocator descriptorAllocator;
vkcomponent::DescriptorLayoutCache descriptorLayoutCache;

std::unordered_map<std::string, DescriptorSetLayoutInfo> descriptorSetLayout;
std::vector<std::string> descriptorLayoutNames;

std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

std::unordered_map<std::string, VkSampler> samplers;

std::string currentlyBoundShader;





VkRenderPass& VulkanContext::GetRenderpass()
{
    
    return *FindUnorderdMap("main pass", renderPass);
}



void VulkanContext::InitVulkan()
{
    VkDeviceManager::InitDevice();
    VkSwapChainManager::InitSwapchain();
    VkCommandbufferManager::InitCommands();
    VkCommandbufferManager::InitSyncStructures();

    descriptorAllocator.Init(VkDeviceManager::GetDevice());
    descriptorLayoutCache.Init(VkDeviceManager::GetDevice());


    mainDeletionQueue.PushFunction([=]() {
        descriptorAllocator.CleanUp();
        //descriptorLayoutCache.CleanUp();
        VkCommandbufferManager::CleanUpCommands();
        VkDeviceManager::DestroyDevice();
    });
    swapDeletionQueue.PushFunction([=]() {
        VkSwapChainManager::DeleteSwapchain();
    });
}
void VulkanContext::ResizeWindow()
{
    //we delete everything that was in the swapchain queue
    vkDeviceWaitIdle(VkDeviceManager::GetDevice());
    swapDeletionQueue.Flush();
    int width = 0, height = 0;
    glfwGetFramebufferSize(windowHandler.p_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(windowHandler.p_window, &width, &height);
        glfwWaitEvents();
    }

    //then we recreate and move deletion functions back to swapchain queue
    VkSwapChainManager::InitSwapchain();
    CreateFramebuffer("", nullptr);

    swapDeletionQueue.PushFunction([=]() {
        VkSwapChainManager::DeleteSwapchain();
    });
    ;   
}

void VulkanContext::UpdateDraw(float clearColor[4],std::function<void()>&& drawCalls)
{
    VkCommandbufferManager::BeginCommands(ResizeWindow);
    BeginRenderpass(clearColor);
    VkViewport viewport = {};
    viewport.width = VkSwapChainManager::GetSwapchainExtent().width;
    viewport.height = VkSwapChainManager::GetSwapchainExtent().height;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor = {};
    scissor.extent = VkSwapChainManager::GetSwapchainExtent();
    scissor.offset = {0, 0};

    vkCmdSetViewport(VkCommandbufferManager::GetCommandBuffer(), 0, 1, &viewport);
    vkCmdSetScissor(VkCommandbufferManager::GetCommandBuffer(), 0, 1, &scissor);

    drawCalls();
    EndRenderpass();
    VkCommandbufferManager::EndCommands(ResizeWindow);
}

void VulkanContext::CleanUpVulkan(FunctionQueuer* p_additionalDeletion)
{
    vkDeviceWaitIdle(VkDeviceManager::GetDevice());
    p_additionalDeletion->Flush();
    for(int i = 0; i < descriptorLayoutNames.size(); i++)
    {
        vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderdMap(descriptorLayoutNames[i], descriptorSetLayout)->layout, nullptr);
    }
    swapDeletionQueue.Flush();
    mainDeletionQueue.Flush();
    
}

void VulkanContext::CreateRenderpass(const std::string& passName)
{
    
    if(passName == "")
    {
        //we define an attachment description for our main color image
        //the attachment is loaded as "clear" when renderpass start
        //the attachment is stored when renderpass ends
        //the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
        //we dont care about stencil, and dont use multisampling
        std::array<VkAttachmentDescription, 2> attachmentDescriptions = {};
        for(int i = 0; i < attachmentDescriptions.size(); i++)
        {
            attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        }

        VkAttachmentDescription color_attachment = {};
        attachmentDescriptions[0].format = VkSwapChainManager::GetSwapchainImageFormat();
        attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachmentDescriptions[1].format = VkSwapChainManager::GetSwapchainDepthFormat();
        attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        //we are going to create 1 subpass, which is the minimum you can do
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        //hook the depth attachment into the subpass
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        //1 dependency, which is from "outside" into the subpass. And we can read or write color
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        //2 attachments from said array
        render_pass_info.attachmentCount = 2;
        render_pass_info.pAttachments = &attachmentDescriptions[0];
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;
        
        renderPass["main pass"];
        VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &render_pass_info, nullptr, FindUnorderdMap("main pass", renderPass)));
        
        mainDeletionQueue.PushFunction([=]() {
            vkDestroyRenderPass(VkDeviceManager::GetDevice(), *FindUnorderdMap("main pass", renderPass), nullptr);
        });
    }
    else {
        renderPass[passName];
        //VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &render_pass_info, nullptr, FindUnorderdMap("main pass", renderPass)));
        ENGINE_CORE_INFO("{0} renderpass created", passName);
    } 
}

void VulkanContext::CreateFramebuffer(const std::string& bufferName, std::unique_ptr<VkFrameBufferAdditionalInfo> bufferInfo)
{
    if(bufferInfo == nullptr && bufferName == "")
    {
        const uint32_t swapchainImageCount = VkSwapChainManager::GetSwapchainImageViews().size();
        frameBuffers.resize(swapchainImageCount);
        
        for (int i = 0; i < swapchainImageCount; i++) {
            frameBuffers[i]["main framebuffer"];
            std::vector <VkImageView> attachments = {VkSwapChainManager::GetSwapchainImageViews()[i], VkSwapChainManager::GetSwapchainDepthView()};
            VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(*FindUnorderdMap("main pass", renderPass), VkSwapChainManager::GetSwapchainExtent());
            fbInfo.attachmentCount = attachments.size();
            fbInfo.pAttachments = attachments.data();
            vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbInfo, nullptr, FindUnorderdMap("main framebuffer", frameBuffers[i]));

            swapDeletionQueue.PushFunction([=]() {
                vkDestroyFramebuffer(VkDeviceManager::GetDevice(), *FindUnorderdMap("main framebuffer", frameBuffers[i]), nullptr);
            });
        }
    }
    else {
        frameBuffers.resize(1);
        frameBuffers[0][bufferName];
        std::vector <VkImageView> attachments;
		// attachments[0] = offscreenPass.color.view;
		// attachments[1] = offscreenPass.depth.view;
        VkExtent2D extent = {};
        extent.height = bufferInfo->height;
        extent.width = bufferInfo->width;
		VkFramebufferCreateInfo fbufCreateInfo = vkinit::FramebufferCreateInfo(*FindUnorderdMap(bufferInfo->renderPass, renderPass), extent);
		fbufCreateInfo.attachmentCount = attachments.size();
		fbufCreateInfo.pAttachments = attachments.data();
		fbufCreateInfo.layers = 1;

		vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbufCreateInfo, nullptr, FindUnorderdMap(bufferName, frameBuffers[0]));
    }
}


void VulkanContext::CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo)
{
    descriptorSetLayoutBindings.push_back(vkinit::DescriptorsetLayoutBinding(layoutBindingInfo.descriptorType,layoutBindingInfo.shaderStageFlags,layoutBindingInfo.binding, layoutBindingInfo.descriptorCount));
}

void VulkanContext::CreateDescriptorSetLayout(const std::string& layoutName)
{
    descriptorSetLayout[layoutName];
    VkDescriptorSetLayoutCreateInfo set = vkinit::DescriptorLayoutInfo(descriptorSetLayoutBindings);
    
    FindUnorderdMap(layoutName, descriptorSetLayout)->layout = descriptorLayoutCache.CreateDescriptorLayout(&set);
    FindUnorderdMap(layoutName, descriptorSetLayout)->bindings = descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.clear();

    descriptorLayoutNames.push_back(layoutName);
}

void VulkanContext::RemoveDescriptorSetLayout(const std::string& layoutName)
{
    vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderdMap(layoutName, descriptorSetLayout)->layout, nullptr);
    for(int i = 0; i < descriptorLayoutNames.size(); i++)
    {
        if(descriptorLayoutNames[i] == layoutName)
        {
            descriptorLayoutNames.erase(descriptorLayoutNames.begin() + i);
            descriptorLayoutNames.shrink_to_fit();
        }
    }
    ENGINE_CORE_INFO("descriptor set layout {0} destroyed", layoutName);
    descriptorSetLayout.erase(layoutName);
}

void VulkanContext::CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter, const VkSamplerAddressMode& samplerAddressMode)
{
    if(FindUnorderdMap(samplerName, samplers) == nullptr)
    {
        VkSamplerCreateInfo samplerInfo = vkinit::SamplerCreateInfo(samplerFilter, samplerAddressMode);

        vkCreateSampler(VkDeviceManager::GetDevice(), &samplerInfo, nullptr, &samplers[samplerName]);
    }
    else {
        ENGINE_CORE_INFO("sampler {0} will be reused", samplerName);
    }
}

void VulkanContext::DestroySampler(const std::string& samplerName)
{
    if(FindUnorderdMap(samplerName, samplers) != nullptr)
    {
        vkDestroySampler(VkDeviceManager::GetDevice(), *FindUnorderdMap(samplerName, samplers), nullptr);
        samplers.erase(samplerName);
    }
}

void VulkanContext::CreateDescriptorSetImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views,const VkFormat& imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{
    std::vector<VkDescriptorImageInfo> imageInfo;
    for(int i = 0; i < views.size(); i++)
    {
        VkDescriptorImageInfo info{};
        info.sampler = *FindUnorderdMap(sampler, samplers);
        info.imageView = views[i];
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.push_back(info);
    }
    auto& bindings = FindUnorderdMap(layoutName, descriptorSetLayout)->bindings;
    
    if(binding == 0)
    {
        descriptorSets[descriptorName];
        descriptorAllocator.Allocate(&FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);
    }
    VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(bindings[binding].descriptorType, FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet, imageInfo.data(), bindings[binding].binding, bindings[binding].descriptorCount);
    vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputTexture, 0, nullptr);
}

void VulkanContext::CreateUniformBufferInfo(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& dataRange)
{
    if(frameOverlap)
    {
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            VkCommandbufferManager::GetFrames(i).allocatedBuffer[bufferName];
            auto& allocatedBuffer = *FindUnorderdMap(bufferName, VkCommandbufferManager::GetFrames(i).allocatedBuffer);
            allocatedBuffer.bufferUsage = bufferUsage;
            allocatedBuffer.dataSize = dataSize;
            allocatedBuffer.dataRange = dataRange;
        }
    }
    else
    {
        allocatedBuffers[bufferName];
        auto& allocatedBuffer = *FindUnorderdMap(bufferName, allocatedBuffers);
        allocatedBuffer.bufferUsage = bufferUsage;
        allocatedBuffer.dataSize = dataSize;
        allocatedBuffer.dataRange = dataRange;
    }

    ENGINE_CORE_INFO("buffer: {0} created", bufferName);
    
}

void VulkanContext::CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding, const bool& frameOverlap ,const std::string& bufferName, const size_t& byteOffset)
{
    auto& bindings = FindUnorderdMap(layoutName, descriptorSetLayout)->bindings;

    VkDescriptorBufferInfo bufferInfo;
    if(frameOverlap == true)
    {
        //with frame overlap
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            auto& frame = VkCommandbufferManager::GetFrames(i);
            auto& buffer = *FindUnorderdMap(bufferName, frame.allocatedBuffer);
            if(FindUnorderdMap(descriptorName, frame.descriptorSets) == nullptr)
            {
                frame.descriptorSets[descriptorName];
                descriptorAllocator.Allocate(&FindUnorderdMap(descriptorName, frame.descriptorSets)->descriptorSet ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);

                if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }
                else
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }

                VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderdMap(descriptorName, frame.descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
                FindUnorderdMap(descriptorName,frame.descriptorSets)->type = bindings[binding].descriptorType;
                vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
            }
            else {
                if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }
                else
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }

                VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderdMap(descriptorName, frame.descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
                for(int i = 0; i < bindings.size(); i++)
                {
                    if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                    {
                        FindUnorderdMap(descriptorName,frame.descriptorSets)->type = bindings[binding].descriptorType;
                    }
                }
                vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
            }
        }
    }
    
    else
    {
        //without frame overlap
        auto& buffer =  *FindUnorderdMap(bufferName, allocatedBuffers);
        if(FindUnorderdMap(descriptorName, descriptorSets) == nullptr)
        {
            descriptorSets[descriptorName];
            descriptorAllocator.Allocate(&FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);

            if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }
            else
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }

            VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
            FindUnorderdMap(descriptorName,descriptorSets)->type = bindings[binding].descriptorType;
            vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
        }
        else {
            if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }
            else
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }

            VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
            for(int i = 0; i < bindings.size(); i++)
            {
                if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    FindUnorderdMap(descriptorName,descriptorSets)->type = bindings[binding].descriptorType;
                }
            }
            vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
        }
    }
    
}

void VulkanContext::RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap)
{
    if(frameOverlap)
    {
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            auto& allocatedBuffer= VkCommandbufferManager::GetFrames(i).allocatedBuffer;
            if( FindUnorderdMap(bufferName, allocatedBuffer) != nullptr)
            {
                vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderdMap(bufferName, allocatedBuffer)->buffer, FindUnorderdMap(bufferName, allocatedBuffer)->allocation);
                allocatedBuffer.erase(bufferName);
            }
        }
    }
    else
    {
        if(FindUnorderdMap(bufferName, allocatedBuffers) != nullptr)
        {
            vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderdMap(bufferName, allocatedBuffers)->buffer, FindUnorderdMap(bufferName, allocatedBuffers)->allocation);
            allocatedBuffers.erase(bufferName);
        }
    }

    ENGINE_CORE_INFO("buffer: {0} removed", bufferName);
}


void VulkanContext::CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkShaderDescriptions& descriptions)
{
    vkcomponent::PipelineBuilder pipelineBuilder;

    std::vector<vkcomponent::ShaderModule> shaderModules;
    shaderModules.resize(shaderPaths.size());
    for(int i = 0; i < shaderPaths.size(); i++)
    {
        vkcomponent::LoadShaderModule(vkcomponent::CompileGLSL(shaderPaths[i]).c_str(), &shaderModules[i], VkDeviceManager::GetDevice());
    }
    vkcomponent::ShaderEffect shaderEffect;
    VkPipelineLayoutCreateInfo pipLayoutInfo = vkinit::PipelineLayoutCreateInfo();
    std::vector<VkDescriptorSetLayout> layouts;
    for(int i = 0; i < layoutNames.size(); i++)
    {
        layouts.push_back(FindUnorderdMap(layoutNames[i], descriptorSetLayout)->layout);
    }
    VkPushConstantRange pushConstantRange = {};
    if(descriptions.p_pushConstant != nullptr)
    {
        pushConstantRange.offset = descriptions.p_pushConstant->offset;
        pushConstantRange.size = descriptions.p_pushConstant->dataSize;
        pushConstantRange.stageFlags = descriptions.p_pushConstant->shaderStage;
        pipLayoutInfo.pushConstantRangeCount = 1;
        pipLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    pipLayoutInfo.pSetLayouts = layouts.data();
    pipLayoutInfo.setLayoutCount = layouts.size();
    shaderEffect = vkcomponent::BuildEffect(shaderModules, pipLayoutInfo);

    pipelineBuilder.pipelineLayout = shaderEffect.builtLayout;
    shaderEffect.FlushLayout();

    //configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(descriptions.polygonMode, descriptions.cullMode);

    //we dont use multisampling, so just run the default one
    pipelineBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

    //a single blend attachment with no blending and writing to RGBA
    if(descriptions.colorBlending)
    {
        pipelineBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();
    }

    //default depthtesting
    pipelineBuilder.depthStencil = vkinit::DepthStencilCreateInfo(descriptions.depthTesting, descriptions.depthTesting, descriptions.depthCompareType);

    //vertex input controls how to read vertices from vertex buffers. We arent using it yet
    pipelineBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

    //input assembly is the configuration for drawing triangle lists, strips, or individual points.
    //we are just going to draw triangle list
    pipelineBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VertexInputDescription vertexDescription = VkVertex::GetVertexDescription(descriptions.vertexLocations);

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();


    //build the pipeline
    vkcomponent::ShaderPass shaderPass;
    // if(renderPass != VK_NULL_HANDLE)
    // {
    //     shaderPass = vkcomponent::BuildShader(renderPass, pipelineBuilder, shaderEffect);
    // }
    // else
    // {
        
    // }
    shaderPass = vkcomponent::BuildShader(*FindUnorderdMap("main pass", renderPass), pipelineBuilder, shaderEffect);

    shaderProgram[shaderName].pass = shaderPass;

    mainDeletionQueue.PushFunction([=](){
        vkDestroyPipeline(VkDeviceManager::GetDevice(), shaderPass.pipeline, nullptr);
        vkDestroyPipelineLayout(VkDeviceManager::GetDevice(), shaderPass.layout, nullptr);
    });
}

void VulkanContext::BindGraphicsPipeline(const std::string& shaderName)
{
    currentlyBoundShader = shaderName;
    if(FindUnorderdMap(currentlyBoundShader, shaderProgram) == nullptr)
    {
        ENGINE_CORE_ERROR("Currently bound shader {0} does not exist");
    }
    else {
        vkCmdBindPipeline(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.pipeline);
    }
}
void VulkanContext::PushConstants(const VkShaderStageFlags& shaderStage, const uint32_t& offset, const uint32_t& dataSize, const void* data)
{
    vkCmdPushConstants(VkCommandbufferManager::GetCommandBuffer(), FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, shaderStage, offset, dataSize, data);
}
void VulkanContext::BindDescriptorSet(const std::string& descriptorName, const uint32_t& set, const uint32_t& dynamicOffset, const bool& frameOverlap)
{
    if(frameOverlap == true)
    {
        if(FindUnorderdMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || FindUnorderdMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
        {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderdMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->descriptorSet, 1,&dynamicOffset);
        }
        else {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderdMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->descriptorSet, 0, 0);
        }
    }
    else {
        if(FindUnorderdMap(descriptorName, descriptorSets)->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || FindUnorderdMap(descriptorName, descriptorSets)->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
        {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet, 1,&dynamicOffset);
        }
        else {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderdMap(descriptorName, descriptorSets)->descriptorSet, 0, 0);
        }
    }
}

void VulkanContext::BindIndexBuffer(AllocatedBuffer& indexBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindIndexBuffer(VkCommandbufferManager::GetCommandBuffer(), indexBuffer.buffer,offset, VK_INDEX_TYPE_UINT32);
}
void VulkanContext::BindVertexBuffer(AllocatedBuffer& vertexBuffer)
{
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(VkCommandbufferManager::GetCommandBuffer(), 0, 1, &vertexBuffer.buffer, &offset);
}

void VulkanContext::PrepareIndirectDraw(const uint32_t& MAX_COMMANDS)
{
    for(int i = 0; i < FRAME_OVERLAP; i++)
    {
        CreateBufferInfo info;
        info.allocSize = MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
        info.bufferUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        info.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        CreateBuffer(VkDeviceManager::GetAllocator(), &VkCommandbufferManager::GetFrames(i).allocatedBuffer["indirect draw"], info);
    }
}
void VulkanContext::UploadIndirectDraw(const uint32_t& objectCount, const std::vector<uint32_t>& indexSize, const uint32_t& currentInstance)
{
    std::vector<VkDrawIndexedIndirectCommand> commands;
    commands.reserve(objectCount);
    for(size_t i = 0; i < objectCount; i++)
    {
    
        VkDrawIndexedIndirectCommand cmd;
        cmd.indexCount = indexSize[i];
        cmd.instanceCount = 1;
        cmd.firstIndex = 0;
        cmd.firstInstance = i;
        commands.push_back(cmd);

    }

	UploadVectorData(FindUnorderdMap("indirect draw", VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->allocation, commands);
}

void VulkanContext::DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex)
{
    uint32_t stride = sizeof(VkDrawIndexedIndirectCommand);
	uint32_t offset = drawIndex * stride;
    vkCmdDrawIndexedIndirect(VkCommandbufferManager::GetCommandBuffer(), FindUnorderdMap("indirect draw", VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->buffer, offset, drawCount, stride);
}

void VulkanContext::DrawIndexed(std::vector<std::uint32_t>& indices, const uint32_t& currentInstance)
{   
    vkCmdDrawIndexed(VkCommandbufferManager::GetCommandBuffer(), indices.size(), 1,0,0,currentInstance);
}

void VulkanContext::BeginRenderpass(const float clearColor[4])
{
    VkClearValue clearValue;
    clearValue.color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };

    //clear depth at 1
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;
    
    //start the main renderpass. 
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo rpInfo;
    // if(renderPass != VK_NULL_HANDLE)
    // {
    //     rpInfo = vkinit::RenderpassBeginInfo(renderPass, VkSwapChainManager::GetSwapchainExtent(), mainFramebuffer[VkCommandbufferManager::GetImageIndex()]);
    // }
    // else
    // {
        
    // }
    rpInfo = vkinit::RenderpassBeginInfo(*FindUnorderdMap("main pass", renderPass), VkSwapChainManager::GetSwapchainExtent(), *FindUnorderdMap("main framebuffer", frameBuffers[VkCommandbufferManager::GetImageIndex()]));
    //connect clear values
    rpInfo.clearValueCount = 2;

    VkClearValue clearValues[] = { clearValue, depthClear };

    rpInfo.pClearValues = &clearValues[0];
    
    vkCmdBeginRenderPass(VkCommandbufferManager::GetCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    
}
void VulkanContext::EndRenderpass()
{
    //finalize the render pass
    vkCmdEndRenderPass(VkCommandbufferManager::GetCommandBuffer());
}

