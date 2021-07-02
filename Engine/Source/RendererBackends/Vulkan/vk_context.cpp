#include "Include/vk_context.h"
#include "Include/vk_device.h"
#include "Include/vk_swapchain.h"
#include "Include/vk_commandbuffer.h"
#include "Include/vk_init.h"

#include "vk_check.h"
#include "vk_types.h"
#include "vk_utils.h"
#include "window_handler.h"

#include "logger.h"
#include <iostream>
#include <unordered_map>
#include <vulkan/vulkan_core.h>





std::unordered_map<std::string, VkRenderPass> renderPass;
std::vector<std::string> renderPassNames;
std::unordered_map<std::string, VkRenderPassInfo>renderPassInfo;
std::unordered_map<std::string, VkFrameBufferAdditionalInfo>frameBuffers;
std::vector<std::string> resizableFramebuffers;

//objects deleted on application closed
FunctionQueuer mainDeletionQueue;
//objects deleted on application window resize
FunctionQueuer swapDeletionQueue;

std::unordered_map<std::string, ShaderProgram> shaderProgram;
std::unordered_map<std::string, DescriptorSetInfo> descriptorSets;

vkcomponent::DescriptorAllocator descriptorAllocator;
vkcomponent::DescriptorLayoutCache descriptorLayoutCache;

std::unordered_map<std::string, DescriptorSetLayoutInfo> descriptorSetLayout;

std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

std::unordered_map<std::string, VkSampler> samplers;

std::string currentlyBoundShader;





VkRenderPass& VulkanContext::GetRenderpass()
{
    
    return *FindUnorderedMap("main pass", renderPass);
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
        descriptorLayoutCache.CleanUp();
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
    for(auto& currentBuffer : resizableFramebuffers)
    {
        CreateFramebuffer(currentBuffer, nullptr);
    }

    swapDeletionQueue.PushFunction([=]() {
        VkSwapChainManager::DeleteSwapchain();
    });
    ;   
}

void VulkanContext::UpdateDraw()
{
    VkCommandbufferManager::BeginCommands(ResizeWindow);
    for(int i = renderPassNames.size()-1; i > -1;i--)
    {
        auto& currentPass = *FindUnorderedMap(renderPassNames[i], renderPassInfo);
        auto& currentBuffer = *FindUnorderedMap(FindUnorderedMap(renderPassNames[i], renderPassInfo)->frameBufferName, frameBuffers);
        VkRenderPassBeginInfo rpInfo = {};
        if(currentPass.frameBufferName != "main framebuffer" && renderPassNames[i] != "main pass")
        {
            for(int j = 0; j < currentBuffer.frameBuffers.size(); j++)
            {
                rpInfo = vkinit::RenderpassBeginInfo(*FindUnorderedMap(renderPassNames[i], renderPass), {currentBuffer.width, currentBuffer.height}, currentBuffer.frameBuffers[j]);
            }
        }
        else {
            rpInfo = vkinit::RenderpassBeginInfo(*FindUnorderedMap(renderPassNames[i], renderPass), VkSwapChainManager::GetSwapchainExtent(), currentBuffer.frameBuffers[VkCommandbufferManager::GetImageIndex()]);
        }
        rpInfo.clearValueCount = currentPass.clearValues.size();

        rpInfo.pClearValues = currentPass.clearValues.data();
        vkCmdBeginRenderPass(VkCommandbufferManager::GetCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport = {};
        VkRect2D scissor = {};
        viewport.width = currentBuffer.width;
        viewport.height = currentBuffer.height;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        scissor.extent = {currentBuffer.width, currentBuffer.height};
        scissor.offset = {0, 0};

        vkCmdSetViewport(VkCommandbufferManager::GetCommandBuffer(), 0, 1, &viewport);
        vkCmdSetScissor(VkCommandbufferManager::GetCommandBuffer(), 0, 1, &scissor);

        currentPass.passQueue.Flush(true);

        vkCmdEndRenderPass(VkCommandbufferManager::GetCommandBuffer());
    }
    VkCommandbufferManager::EndCommands(ResizeWindow);
}

void VulkanContext::CleanUpVulkan(FunctionQueuer* p_additionalDeletion)
{
    vkDeviceWaitIdle(VkDeviceManager::GetDevice());
    p_additionalDeletion->Flush();
    swapDeletionQueue.Flush();
    mainDeletionQueue.Flush();
    
}
void VulkanContext::CreateFramebuffer(const std::string& bufferName, std::unique_ptr<VkFrameBufferAdditionalInfo> p_bufferInfo)
{
    if(p_bufferInfo == nullptr && bufferName == "")
    {
        if(FindUnorderedMap("main pass", renderPass) == nullptr)
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
            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            //2 attachments from said array
            renderPassInfo.attachmentCount = 2;
            renderPassInfo.pAttachments = &attachmentDescriptions[0];
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;
            
            renderPass["main pass"];
            renderPassNames.push_back("main pass");
            VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &renderPassInfo, nullptr, FindUnorderedMap("main pass", renderPass)));
            
            mainDeletionQueue.PushFunction([=]() {
                vkDestroyRenderPass(VkDeviceManager::GetDevice(), *FindUnorderedMap("main pass", renderPass), nullptr);
            });
        }

        if(FindUnorderedMap("main framebuffer",frameBuffers) == nullptr)
        {
            resizableFramebuffers.push_back(bufferName);
            frameBuffers["main framebuffer"];
        }
        FindUnorderedMap("main framebuffer",frameBuffers)->frameBuffers.resize(VkSwapChainManager::GetSwapchainImageViews().size());
        auto& bufferInfo = *FindUnorderedMap("main framebuffer",frameBuffers);
        bufferInfo.width = VkSwapChainManager::GetSwapchainExtent().width;
        bufferInfo.height = VkSwapChainManager::GetSwapchainExtent().height;
        for (int i = 0; i < bufferInfo.frameBuffers.size(); i++) {
            std::vector <VkImageView> attachments = {VkSwapChainManager::GetSwapchainImageViews()[i], VkSwapChainManager::GetSwapchainDepthView()};
            VkFramebufferCreateInfo fbInfo = vkinit::FramebufferCreateInfo(*FindUnorderedMap("main pass", renderPass), VkSwapChainManager::GetSwapchainExtent());
            fbInfo.attachmentCount = attachments.size();
            fbInfo.pAttachments = attachments.data();
            vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbInfo, nullptr, &bufferInfo.frameBuffers[i]);

            swapDeletionQueue.PushFunction([=]() {
                vkDestroyFramebuffer(VkDeviceManager::GetDevice(), bufferInfo.frameBuffers[i], nullptr);
            });
        }
    }
    else {

        frameBuffers[bufferName];
        auto& fbufferInfo = *FindUnorderedMap(bufferName,frameBuffers);
        fbufferInfo = *p_bufferInfo;
        fbufferInfo.frameBuffers.resize(1);
        if(FindUnorderedMap(fbufferInfo.renderPass, renderPass) == nullptr)
        {
            //for now a lot of stuff is hard coded
            std::vector<VkAttachmentDescription> attachmentDescriptions = {};
            attachmentDescriptions.resize(fbufferInfo.images.size());
            std::vector<VkAttachmentReference> attachmentRefs;
            attachmentRefs.resize(fbufferInfo.images.size());
            for(int i = 0; i < attachmentDescriptions.size(); i++)
            {
                attachmentDescriptions[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDescriptions[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
                if(fbufferInfo.attachmentRefs[i].layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                {
                    attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
                else {
                    attachmentDescriptions[i].finalLayout = fbufferInfo.attachmentRefs[i].layout;
                }
                attachmentRefs[i].attachment = i;
                attachmentRefs[i].layout = fbufferInfo.attachmentRefs[i].layout;
                if(fbufferInfo.imageFormats[i] == VK_FORMAT_UNDEFINED && fbufferInfo.attachmentRefs[i].layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                {
                    fbufferInfo.imageFormats[i] = vkinit::GetSupportedDepthFormat(VkDeviceManager::GetPhysicalDevice());
                    attachmentDescriptions[i].format = fbufferInfo.imageFormats[i];
                }
                else {
                    attachmentDescriptions[i].format = fbufferInfo.imageFormats[i];
                }
            }

            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            std::array<VkSubpassDependency, 2> dependencies;
            for(int i = 0; i < attachmentRefs.size(); i++)
            {
                if(attachmentRefs[i].layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                {
                    subpassDescription.colorAttachmentCount += 1;
                    subpassDescription.pColorAttachments = &attachmentRefs[i];
                }
                else {
                    subpassDescription.pDepthStencilAttachment = &attachmentRefs[i];
                }
            }

            // Use subpass dependencies for layout transitions
            if(attachmentRefs.size() > 1)
            {
                dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[0].dstSubpass = 0;
                dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                dependencies[1].srcSubpass = 0;
                dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }
            else {
                if(attachmentRefs[0].layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                {
                    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[0].dstSubpass = 0;
                    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

                    dependencies[1].srcSubpass = 0;
                    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                }
            }

            // Create the actual renderpass
            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
            renderPassInfo.pAttachments = attachmentDescriptions.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();
            renderPass[fbufferInfo.renderPass];
            renderPassNames.push_back(fbufferInfo.renderPass);
            VK_CHECK(vkCreateRenderPass(VkDeviceManager::GetDevice(), &renderPassInfo, nullptr, FindUnorderedMap(fbufferInfo.renderPass, renderPass)));
            ENGINE_CORE_INFO("{0} renderpass created", fbufferInfo.renderPass);
            mainDeletionQueue.PushFunction([=]() {
                vkDestroyRenderPass(VkDeviceManager::GetDevice(), *FindUnorderedMap(fbufferInfo.renderPass, renderPass), nullptr);
            });
        }
        
        //create images for buffer
        VkExtent3D extent3D;
        extent3D.width = fbufferInfo.width;
        extent3D.height = fbufferInfo.height;
        extent3D.depth = 1.0f;
        std::vector <VkImageView> attachments;

        for(int i = 0; i < fbufferInfo.images.size(); i++)
        {
            VkImageCreateInfo imageInfo = {};
            if(fbufferInfo.attachmentRefs[i].layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                imageInfo = vkinit::ImageCreateInfo(fbufferInfo.imageFormats[i], VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, extent3D);
            }
            else {
                imageInfo = vkinit::ImageCreateInfo(fbufferInfo.imageFormats[i], VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, extent3D);
            }
            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(VkDeviceManager::GetAllocator(),&imageInfo, &dimg_allocinfo, &fbufferInfo.images[i].image, &fbufferInfo.images[i].allocation, nullptr);

            VkImageViewCreateInfo imageViewInfo = {};
            if(fbufferInfo.attachmentRefs[i].layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                imageViewInfo = vkinit::ImageViewCreateInfo(fbufferInfo.imageFormats[i], fbufferInfo.images[i].image, VK_IMAGE_ASPECT_COLOR_BIT);
            }
            else {
                imageViewInfo = vkinit::ImageViewCreateInfo(fbufferInfo.imageFormats[i], fbufferInfo.images[i].image, VK_IMAGE_ASPECT_DEPTH_BIT);
            }
            vkCreateImageView(VkDeviceManager::GetDevice(), &imageViewInfo, nullptr, &fbufferInfo.images[i].defaultView);
            attachments.push_back(fbufferInfo.images[i].defaultView);
        }
        VkExtent2D extent = {};
        extent.height = extent3D.height;
        extent.width = extent3D.width;
		VkFramebufferCreateInfo fbufCreateInfo = vkinit::FramebufferCreateInfo(*FindUnorderedMap(fbufferInfo.renderPass, renderPass), extent);
		fbufCreateInfo.attachmentCount = attachments.size();
		fbufCreateInfo.pAttachments = attachments.data();
		fbufCreateInfo.layers = 1;

		vkCreateFramebuffer(VkDeviceManager::GetDevice(), &fbufCreateInfo, nullptr, &fbufferInfo.frameBuffers[0]);
        //Check if we want to resize the framebuffer
        //At the moment the implementation isn't flexible 
        if(fbufferInfo.resizable == true)
        {
            resizableFramebuffers.push_back(bufferName);
            swapDeletionQueue.PushFunction([=]() {
                vkDestroyFramebuffer(VkDeviceManager::GetDevice(), fbufferInfo.frameBuffers[0], nullptr);
            });
        }
        else {
            mainDeletionQueue.PushFunction([=]() {
                vkDestroyFramebuffer(VkDeviceManager::GetDevice(), fbufferInfo.frameBuffers[0], nullptr);
            });  
        }
        mainDeletionQueue.PushFunction([=]() {
            for(int i = 0; i < attachments.size(); i++)
            {
                vkDestroyImageView(VkDeviceManager::GetDevice(), attachments[i],nullptr);
                vmaDestroyImage(VkDeviceManager::GetAllocator(), fbufferInfo.images[i].image,  fbufferInfo.images[i].allocation);
            }
        }); 
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
    
    FindUnorderedMap(layoutName, descriptorSetLayout)->layout = descriptorLayoutCache.CreateDescriptorLayout(&set);
    FindUnorderedMap(layoutName, descriptorSetLayout)->bindings = descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.clear();

    // mainDeletionQueue.PushFunction([=]{
    //     if(FindUnorderedMap(layoutName, descriptorSetLayout) != nullptr)
    //     {
    //         vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderedMap(layoutName, descriptorSetLayout)->layout, nullptr);
    //         ENGINE_CORE_ERROR(layoutName);
    //         descriptorSetLayout.erase(layoutName);
    //     }
    // });

}

void VulkanContext::RemoveDescriptorSetLayout(const std::string& layoutName)
{
    vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderedMap(layoutName, descriptorSetLayout)->layout, nullptr);
    ENGINE_CORE_INFO("descriptor set layout {0} destroyed", layoutName);
    descriptorSetLayout.erase(layoutName);
}

void VulkanContext::CreateSampler(const std::string& samplerName, const VkFilter& samplerFilter, const VkSamplerAddressMode& samplerAddressMode)
{
    if(FindUnorderedMap(samplerName, samplers) == nullptr)
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
    if(FindUnorderedMap(samplerName, samplers) != nullptr)
    {
        vkDestroySampler(VkDeviceManager::GetDevice(), *FindUnorderedMap(samplerName, samplers), nullptr);
        samplers.erase(samplerName);
    }
}

void VulkanContext::CreateDescriptorSetImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::vector<VkImageView>& views,const VkFormat& imageFormat /*= VK_FORMAT_R8G8B8A8_SRGB*/)
{
    std::vector<VkDescriptorImageInfo> imageInfo;
    for(int i = 0; i < views.size(); i++)
    {
        VkDescriptorImageInfo info{};
        if(FindUnorderedMap(sampler, samplers) != nullptr)
        {
            info.sampler = *FindUnorderedMap(sampler, samplers);
        }
        else {
            CreateSampler("",VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
            info.sampler = *FindUnorderedMap("", samplers);
            mainDeletionQueue.PushFunction([=]{
                vkDestroySampler(VkDeviceManager::GetDevice(), *FindUnorderedMap("", samplers), nullptr);
            });
        }
        info.imageView = views[i];
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.push_back(info);
    }
    auto& bindings = FindUnorderedMap(layoutName, descriptorSetLayout)->bindings;
    
    if(binding == 0)
    {
        descriptorSets[descriptorName];
        descriptorAllocator.Allocate(&FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, FindUnorderedMap(layoutName, descriptorSetLayout)->layout);
    }
    VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, imageInfo.data(), bindings[binding].binding, bindings[binding].descriptorCount);
    vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputTexture, 0, nullptr);
}

void VulkanContext::CreateDescriptorSetFrameBufferImage(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding,const std::string& sampler,const std::string& bufferName)
{
    std::vector<VkDescriptorImageInfo> imageInfo;
    for(int i = 0; i < FindUnorderedMap(bufferName, frameBuffers)->images.size(); i++)
    {
        if(FindUnorderedMap(bufferName, frameBuffers)->attachmentRefs[i].layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && FindUnorderedMap(bufferName, frameBuffers)->images.size() > 1)
        {
            VkDescriptorImageInfo info{};
            info.sampler = *FindUnorderedMap(sampler, samplers);
            info.imageView = FindUnorderedMap(bufferName, frameBuffers)->images[i].defaultView;
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.push_back(info);
        }
    }
    
    auto& bindings = FindUnorderedMap(layoutName, descriptorSetLayout)->bindings;
    
    if(binding == 0)
    {
        descriptorSets[descriptorName];
        descriptorAllocator.Allocate(&FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet ,FindUnorderedMap(layoutName, descriptorSetLayout)->layout);
    }
    VkWriteDescriptorSet outputTexture = vkinit::WriteDescriptorImage(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, imageInfo.data(), bindings[binding].binding, bindings[binding].descriptorCount);
    vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputTexture, 0, nullptr);
}

void VulkanContext::CreateUniformBufferInfo(const std::string& bufferName, const bool& frameOverlap,const VkBufferUsageFlags& bufferUsage, const size_t& dataSize, const size_t& dataRange)
{
    if(frameOverlap)
    {
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            VkCommandbufferManager::GetFrames(i).allocatedBuffer[bufferName];
            auto& allocatedBuffer = *FindUnorderedMap(bufferName, VkCommandbufferManager::GetFrames(i).allocatedBuffer);
            allocatedBuffer.bufferUsage = bufferUsage;
            allocatedBuffer.dataSize = dataSize;
            allocatedBuffer.dataRange = dataRange;
        }
    }
    else
    {
        allocatedBuffers[bufferName];
        auto& allocatedBuffer = *FindUnorderedMap(bufferName, allocatedBuffers);
        allocatedBuffer.bufferUsage = bufferUsage;
        allocatedBuffer.dataSize = dataSize;
        allocatedBuffer.dataRange = dataRange;
    }

    ENGINE_CORE_INFO("buffer: {0} created", bufferName);
    
}

void VulkanContext::CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName, const uint32_t& binding, const bool& frameOverlap ,const std::string& bufferName, const size_t& byteOffset)
{
    auto& bindings = FindUnorderedMap(layoutName, descriptorSetLayout)->bindings;

    VkDescriptorBufferInfo bufferInfo;
    if(frameOverlap == true)
    {
        //with frame overlap
        for(int i = 0; i < FRAME_OVERLAP; i++)
        {
            auto& frame = VkCommandbufferManager::GetFrames(i);
            auto& buffer = *FindUnorderedMap(bufferName, frame.allocatedBuffer);
            if(FindUnorderedMap(descriptorName, frame.descriptorSets) == nullptr)
            {
                frame.descriptorSets[descriptorName];
                descriptorAllocator.Allocate(&FindUnorderedMap(descriptorName, frame.descriptorSets)->descriptorSet ,FindUnorderedMap(layoutName, descriptorSetLayout)->layout);

                if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }
                else
                {
                    bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
                }

                VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, frame.descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
                FindUnorderedMap(descriptorName,frame.descriptorSets)->type = bindings[binding].descriptorType;
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

                VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, frame.descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
                for(int i = 0; i < bindings.size(); i++)
                {
                    if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                    {
                        FindUnorderedMap(descriptorName,frame.descriptorSets)->type = bindings[binding].descriptorType;
                    }
                }
                vkUpdateDescriptorSets(VkDeviceManager::GetDevice(), 1, &outputBuffer, 0, nullptr);
            }
        }
    }
    
    else
    {
        //without frame overlap
        auto& buffer =  *FindUnorderedMap(bufferName, allocatedBuffers);
        if(FindUnorderedMap(descriptorName, descriptorSets) == nullptr)
        {
            descriptorSets[descriptorName];
            descriptorAllocator.Allocate(&FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet ,FindUnorderedMap(layoutName, descriptorSetLayout)->layout);

            if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }
            else
            {
                bufferInfo = CreateDescriptorBuffer(buffer, buffer.dataSize, byteOffset, buffer.dataRange);
            }

            VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
            FindUnorderedMap(descriptorName,descriptorSets)->type = bindings[binding].descriptorType;
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

            VkWriteDescriptorSet outputBuffer = vkinit::WriteDescriptorBuffer(bindings[binding].descriptorType, FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, &bufferInfo, bindings[binding].binding);
            for(int i = 0; i < bindings.size(); i++)
            {
                if(bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || bindings[binding].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
                {
                    FindUnorderedMap(descriptorName,descriptorSets)->type = bindings[binding].descriptorType;
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
            if( FindUnorderedMap(bufferName, allocatedBuffer) != nullptr)
            {
                vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderedMap(bufferName, allocatedBuffer)->buffer, FindUnorderedMap(bufferName, allocatedBuffer)->allocation);
                allocatedBuffer.erase(bufferName);
            }
        }
    }
    else
    {
        if(FindUnorderedMap(bufferName, allocatedBuffers) != nullptr)
        {
            vmaDestroyBuffer(VkDeviceManager::GetAllocator(), FindUnorderedMap(bufferName, allocatedBuffers)->buffer, FindUnorderedMap(bufferName, allocatedBuffers)->allocation);
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
        layouts.push_back(FindUnorderedMap(layoutNames[i], descriptorSetLayout)->layout);
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
    if(descriptions.vertexLocations.size() > 0)
    {
        //connect the pipeline builder vertex input info to the one we get from Vertex
        pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
        pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

        pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
        pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();
    }


    //build the pipeline
    vkcomponent::ShaderPass shaderPass;
    shaderPass = vkcomponent::BuildShader(*FindUnorderedMap(descriptions.renderPassName, renderPass), pipelineBuilder, shaderEffect);

    shaderProgram[shaderName].pass = shaderPass;

    mainDeletionQueue.PushFunction([=](){
        vkDestroyPipeline(VkDeviceManager::GetDevice(), shaderPass.pipeline, nullptr);
        vkDestroyPipelineLayout(VkDeviceManager::GetDevice(), shaderPass.layout, nullptr);
    });
}

void VulkanContext::BindGraphicsPipeline(const std::string& shaderName)
{
    currentlyBoundShader = shaderName;
    if(FindUnorderedMap(currentlyBoundShader, shaderProgram) == nullptr)
    {
        ENGINE_CORE_ERROR("Currently bound shader {0} does not exist");
    }
    else {
        vkCmdBindPipeline(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.pipeline);
    }
}
void VulkanContext::PushConstants(const VkShaderStageFlags& shaderStage, const uint32_t& offset, const uint32_t& dataSize, const void* data)
{
    vkCmdPushConstants(VkCommandbufferManager::GetCommandBuffer(), FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, shaderStage, offset, dataSize, data);
}
void VulkanContext::BindDescriptorSet(const std::string& descriptorName, const uint32_t& set, const uint32_t& dynamicOffset, const bool& frameOverlap)
{
    if(frameOverlap == true)
    {
        if(FindUnorderedMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets) != nullptr)
        {
            if(FindUnorderedMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || FindUnorderedMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderedMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->descriptorSet, 1,&dynamicOffset);
            }
            else {
                vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderedMap(descriptorName, VkCommandbufferManager::GetCurrentFrame().descriptorSets)->descriptorSet, 0, 0);
            }
        }
        else {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1,VK_NULL_HANDLE, 0, 0);
        }
    }
    else {
        if(FindUnorderedMap(descriptorName, descriptorSets) != nullptr)
        {
            if(FindUnorderedMap(descriptorName, descriptorSets)->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || FindUnorderedMap(descriptorName, descriptorSets)->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
            {
                vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, 1,&dynamicOffset);
            }
            else {
                vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1, &FindUnorderedMap(descriptorName, descriptorSets)->descriptorSet, 0, 0);
            }
        }
        else {
            vkCmdBindDescriptorSets(VkCommandbufferManager::GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderedMap(currentlyBoundShader, shaderProgram)->pass.layout, set, 1,VK_NULL_HANDLE, 0, 0);
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

	UploadVectorData(FindUnorderedMap("indirect draw", VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->allocation, commands);
}

void VulkanContext::DrawIndexedIndirect(const uint32_t& drawCount, const uint32_t& drawIndex)
{
    uint32_t stride = sizeof(VkDrawIndexedIndirectCommand);
	uint32_t offset = drawIndex * stride;
    vkCmdDrawIndexedIndirect(VkCommandbufferManager::GetCommandBuffer(), FindUnorderedMap("indirect draw", VkCommandbufferManager::GetCurrentFrame().allocatedBuffer)->buffer, offset, drawCount, stride);
}

void VulkanContext::Draw(const uint32_t& vertices, const uint32_t& instanceCount, const uint32_t& firstVertex, const uint32_t& firstInstance)
{
    vkCmdDraw(VkCommandbufferManager::GetCommandBuffer(), vertices, instanceCount, firstVertex, firstInstance);
}

void VulkanContext::DrawIndexed(std::vector<std::uint32_t>& indices, const uint32_t& currentInstance)
{   
    vkCmdDrawIndexed(VkCommandbufferManager::GetCommandBuffer(), indices.size(), 1,0,0,currentInstance);
}

void VulkanContext::PrepareRenderpassForDraw(const float& clearValueCount, const float clearColor[4], const float& depth, const std::string& passName /*="main pass"*/, const std::string& frameBufferName /*="main framebuffer"*/)
{
    if(FindUnorderedMap(passName, renderPassInfo) == nullptr)
    {
        renderPassInfo[passName];
        VkClearValue clearValue;
        clearValue.color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };

        //clear depth at 1
        VkClearValue depthClear;
        depthClear.depthStencil.depth = depth;

        FindUnorderedMap(passName, renderPassInfo)->clearValues.push_back(clearValue);
        if(clearValueCount > 1)
        {
            FindUnorderedMap(passName, renderPassInfo)->clearValues.push_back(depthClear);
        }
        
        FindUnorderedMap(passName, renderPassInfo)->frameBufferName = frameBufferName;
    }
    else {
        VkClearValue clearValue;
        clearValue.color = { {clearColor[0], clearColor[1], clearColor[2], clearColor[3]} };

        //clear depth at 1
        VkClearValue depthClear;
        depthClear.depthStencil.depth = depth;

        FindUnorderedMap(passName, renderPassInfo)->clearValues.push_back(clearValue);
        if(clearValueCount > 1)
        {
            FindUnorderedMap(passName, renderPassInfo)->clearValues.push_back(depthClear);
        }
        
        //start the main renderpass. 
        //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
        

        FindUnorderedMap(passName, renderPassInfo)->frameBufferName = frameBufferName;
    }
    
}
void VulkanContext::AddDrawToRenderpassQueue(std::function<void()>&& drawCalls, const std::string& passName /*="main pass"*/)
{
    auto& pass = *FindUnorderedMap(passName, renderPassInfo);
    pass.passQueue.PushFunction([=]{
        drawCalls();
    });
}

