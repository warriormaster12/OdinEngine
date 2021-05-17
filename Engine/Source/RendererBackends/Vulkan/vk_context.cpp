#include "Include/vk_context.h"
#include "Include/vk_device.h"
#include "Include/vk_swapchain.h"
#include "Include/vk_commandbuffer.h"
#include "Include/vk_init.h"

#include "vk_check.h"
#include "window_handler.h"

#include "logger.h"




VkRenderPass mainPass;
std::vector<VkFramebuffer> mainFramebuffer;

//objects deleted on application closed
FunctionQueuer mainDeletionQueue;
//objects deleted on application window resize
FunctionQueuer swapDeletionQueue;

std::unordered_map<std::string, ShaderProgram> shaderProgram;
std::unordered_map<std::string, VkDescriptorSet> descriptorSets;

vkcomponent::DescriptorAllocator descriptorAllocator;
vkcomponent::DescriptorLayoutCache descriptorLayoutCache;

std::unordered_map<std::string, DescriptorSetLayoutInfo> descriptorSetLayout;
std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

std::unordered_map<std::string, VkSampler> samplers;

std::string currentlyBoundShader;





VkRenderPass& VulkanContext::GetRenderpass()
{
    return mainPass;
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

    swapDeletionQueue.PushFunction([=]() {
        VkSwapChainManager::DeleteSwapchain();
    });
    CreateMainFramebuffer();   
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
    swapDeletionQueue.Flush();
    mainDeletionQueue.Flush();
    
}

void VulkanContext::CreateMainFramebuffer()
{
    const uint32_t swapchainImageCount = VkSwapChainManager::GetSwapchainImageViews().size();
    mainFramebuffer = std::vector<VkFramebuffer>(swapchainImageCount);
    
    
    for (int i = 0; i < swapchainImageCount; i++) {
        std::vector <VkImageView> attachments = {VkSwapChainManager::GetSwapchainImageViews()[i], VkSwapChainManager::GetSwapchainDepthView()};
        VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(mainPass, VkSwapChainManager::GetSwapchainExtent());
        fbInfo.attachmentCount = attachments.size();
        fbInfo.pAttachments = attachments.data();
        vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbInfo, nullptr, &mainFramebuffer[i]);

        swapDeletionQueue.PushFunction([=]() {
            vkDestroyFramebuffer(VkDeviceManager::GetDevice(), mainFramebuffer[i], nullptr);
        });
    }
}

void VulkanContext::CreateDefaultRenderpass()
{
    //we define an attachment description for our main color image
    //the attachment is loaded as "clear" when renderpass start
    //the attachment is stored when renderpass ends
    //the attachment layout starts as "undefined", and transitions to "Present" so its possible to display it
    //we dont care about stencil, and dont use multisampling

    VkAttachmentDescription color_attachment = {};
    color_attachment.format = VkSwapChainManager::GetSwapchainImageFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};
    // Depth attachment
    depth_attachment.flags = 0;
    depth_attachment.format = VkSwapChainManager::GetSwapchainDepthFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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


    //array of 2 attachments, one for the color, and other for depth
    VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //2 attachments from said array
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = &attachments[0];
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &render_pass_info, nullptr, &mainPass));
    ENGINE_CORE_INFO("main renderpass created");
    mainDeletionQueue.PushFunction([=]() {
        vkDestroyRenderPass(VkDeviceManager::GetDevice(), mainPass, nullptr);
    }); 
}

void VulkanContext::CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo)
{
    descriptorSetLayoutBindings.push_back(vkinit::DescriptorsetLayoutBinding(layoutBindingInfo.descriptorType,layoutBindingInfo.shaderStageFlags,layoutBindingInfo.binding, layoutBindingInfo.descriptorCount));
}

void VulkanContext::CreateDescriptorSetLayout(const std::string& layoutName)
{
    VkDescriptorSetLayoutCreateInfo set = vkinit::DescriptorLayoutInfo(descriptorSetLayoutBindings);
    if(FindUnorderdMap(layoutName, descriptorSetLayout) == nullptr)
    {
        //create a new list of known descriptorSetLayouts
        descriptorSetLayout[layoutName].layout = descriptorLayoutCache.CreateDescriptorLayout(&set);
        FindUnorderdMap(layoutName, descriptorSetLayout)->bindings = descriptorSetLayoutBindings;
    }
    else
    {
        //add to existing list
        FindUnorderdMap(layoutName, descriptorSetLayout)->layout = descriptorLayoutCache.CreateDescriptorLayout(&set);
        FindUnorderdMap(layoutName, descriptorSetLayout)->bindings = descriptorSetLayoutBindings;
    }
    descriptorSetLayoutBindings.clear();
}

void VulkanContext::RemoveDescriptorSetLayout(const std::string& layoutName)
{
    vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderdMap(layoutName, descriptorSetLayout)->layout, nullptr);
}

void VulkanContext::CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter, const VkSamplerAddressMode& samplerAddressMode)
{
    VkSamplerCreateInfo samplerInfo = vkinit::SamplerCreateInfo(samplerFilter, samplerAddressMode);

    vkCreateSampler(VkDeviceManager::GetDevice(), &samplerInfo, nullptr, &samplers[samplerName]);
}

void VulkanContext::DestroySampler(const std::string& samplerName)
{
    vkDestroySampler(VkDeviceManager::GetDevice(), *FindUnorderdMap(samplerName, samplers), nullptr);
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
        descriptorAllocator.Allocate(&descriptorSets[descriptorName] ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);
    }

    VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(bindings[binding].descriptorType, *FindUnorderdMap(descriptorName, descriptorSets), imageInfo.data(), bindings[binding].binding, bindings[binding].descriptorCount);

    vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputTexture, 0, nullptr);
}

void VulkanContext::CreateUniformBufferInfo(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& byteOffset)
{
    if(frameOverlap)
    {
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            VkCommandbufferManager::GetFrames(i).allocatedBuffer[bufferName];
            auto& allocatedBuffer = *FindUnorderdMap(bufferName, VkCommandbufferManager::GetFrames(i).allocatedBuffer);
            allocatedBuffer.bufferUsage = bufferUsage;
            allocatedBuffer.dataSize = dataSize;
            allocatedBuffer.byteOffset = byteOffset;
        }
    }
    else
    {
        allocatedBuffers[bufferName];
        auto& allocatedBuffer = *FindUnorderdMap(bufferName, allocatedBuffers);
        allocatedBuffer.bufferUsage = bufferUsage;
        allocatedBuffer.dataSize = dataSize;
        allocatedBuffer.byteOffset = byteOffset;
    }

    ENGINE_CORE_INFO("buffer: {0} created", bufferName);
    
}

void VulkanContext::CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding, const bool& frameOverlap ,const std::string& bufferName)
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
            if(binding == 0)
            {
                descriptorAllocator.Allocate(&frame.descriptorSets[descriptorName] ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);
            }
            if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                bufferInfo = CreateDescriptorBuffer(buffer, FRAME_OVERLAP * PadUniformBufferSize(buffer.dataSize), buffer.byteOffset);
            }
            else
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, buffer.byteOffset);
            }
            
            VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, *FindUnorderdMap(descriptorName, frame.descriptorSets), &bufferInfo, bindings[binding].binding);

            vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
        }
    }
    
    else
    {
        //without frame overlap
        auto& buffer =  *FindUnorderdMap(bufferName, allocatedBuffers);
        if(binding == 0)
        {
            descriptorAllocator.Allocate(&descriptorSets[descriptorName] ,FindUnorderdMap(layoutName, descriptorSetLayout)->layout);
        }
        if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
        {
            bufferInfo = CreateDescriptorBuffer(buffer, FRAME_OVERLAP * PadUniformBufferSize(buffer.dataSize), buffer.byteOffset);
        }
        else
        {
            bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, buffer.byteOffset);
        }

        VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, *FindUnorderdMap(descriptorName, descriptorSets), &bufferInfo, bindings[binding].binding);

        vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
    }
    
}

void VulkanContext::RemoveAllocatedBuffer(const std::string& bufferName, const bool& frameOverlap)
{
    if(frameOverlap)
    {
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            auto& allocatedBuffer= VkCommandbufferManager::GetFrames(i).allocatedBuffer;
            vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderdMap(bufferName, allocatedBuffer)->buffer, FindUnorderdMap(bufferName, allocatedBuffer)->allocation);
        }
    }
    else
    {
        vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderdMap(bufferName, allocatedBuffers)->buffer, FindUnorderdMap(bufferName, allocatedBuffers)->allocation);
    }
}


void VulkanContext::CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkShaderDescriptions& descriptions,const VkRenderPass& renderPass /*= VK_NULL_HANDLE*/)
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
    pipLayoutInfo.pSetLayouts = layouts.data();
    pipLayoutInfo.setLayoutCount = layouts.size();
    shaderEffect = *vkcomponent::BuildEffect(shaderModules, pipLayoutInfo);

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
    if(renderPass != VK_NULL_HANDLE)
    {
        shaderPass = *vkcomponent::BuildShader(renderPass, pipelineBuilder, &shaderEffect);
    }
    else
    {
        shaderPass = *vkcomponent::BuildShader(mainPass, pipelineBuilder, &shaderEffect);
    }

    shaderProgram[shaderName].pass = shaderPass;

    mainDeletionQueue.PushFunction([=](){
        vkDestroyPipeline(VkDeviceManager::GetDevice(), shaderPass.pipeline, nullptr);
        vkDestroyPipelineLayout(VkDeviceManager::GetDevice(), shaderPass.layout, nullptr);
    });
}

void VulkanContext::BindGraphicsPipeline(const std::string& shaderName)
{
    currentlyBoundShader = shaderName;
    vkCmdBindPipeline(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.pipeline);
}
void VulkanContext::BindDescriptorSet(const std::string& descriptorName, const uint32_t& set, const bool& frameOverlap,const bool& isDynamic, const size_t& dataSize)
{
    if(frameOverlap == false && isDynamic == false)
    {
        vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, descriptorSets), 0, 0);
    }
    else if(frameOverlap == true && isDynamic == false)
    {
        auto& currentFrame = VkCommandbufferManager::GetCurrentFrame();
        vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, currentFrame.descriptorSets), 0, 0); 
    }
    else if(frameOverlap == false && isDynamic == true)
    {
        size_t frameIndex = frameNumber % FRAME_OVERLAP;
        uint32_t descriptorOffset = PadUniformBufferSize(dataSize) * frameIndex;
        auto& currentFrame = VkCommandbufferManager::GetCurrentFrame();
        vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, descriptorSets), 1, &descriptorOffset);
    }
    else
    {
        size_t frameIndex = frameNumber % FRAME_OVERLAP;
        uint32_t descriptorOffset = PadUniformBufferSize(dataSize) * frameIndex;
        auto& currentFrame = VkCommandbufferManager::GetCurrentFrame();
        vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, currentFrame.descriptorSets), 1, &descriptorOffset);
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

void VulkanContext::BeginRenderpass(const float clearColor[4], const VkRenderPass& renderPass /*= VK_NULL_HANDLE*/)
{
    VkClearValue clearValue;
    clearValue.color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };

    //clear depth at 1
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;
    
    //start the main renderpass. 
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo rpInfo;
    if(renderPass != VK_NULL_HANDLE)
    {
        rpInfo = vkinit::RenderpassBeginInfo(renderPass, VkSwapChainManager::GetSwapchainExtent(), mainFramebuffer[VkCommandbufferManager::GetImageIndex()]);
    }
    else
    {
        rpInfo = vkinit::RenderpassBeginInfo(mainPass, VkSwapChainManager::GetSwapchainExtent(), mainFramebuffer[VkCommandbufferManager::GetImageIndex()]);
    }
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

