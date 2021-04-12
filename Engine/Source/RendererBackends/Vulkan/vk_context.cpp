#include "Include/vk_context.h"
#include "Include/vk_device.h"
#include "Include/vk_swapchain.h"
#include "Include/vk_commandbuffer.h"
#include "Include/vk_init.h"

#include "vk_check.h"
#include "function_queuer.h"
#include "window_handler.h"



VkRenderPass mainPass;
std::vector<VkFramebuffer> mainFramebuffer;
uint32_t swapchainImageIndex;
VkCommandBuffer cmd;

//objects deleted on application closed
FunctionQueuer mainDeletionQueue;
//objects deleted on application window resize
FunctionQueuer swapDeletionQueue;

std::unordered_map<std::string, ShaderProgram> shaderProgram;
std::unordered_map<std::string, VkDescriptorSet> descriptorSets;

vkcomponent::DescriptorAllocator descriptorAllocator;
vkcomponent::DescriptorLayoutCache descriptorLayoutCache;

std::unordered_map<std::string, DescriptorSetInfo> descriptorSetLayout;
std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;



namespace
{  
    //find unordered_map
    template<typename T>
    T* FindUnorderdMap(const std::string& name, std::unordered_map<std::string, T>& data)
    {
        //search for the object, and return nullpointer if not found
        auto it = data.find(name);
        if (it == data.end()) {
            return nullptr;
        }
        else {
            return &it->second;
        }
    }
}


namespace VulkanContext 
{
    void InitVulkan()
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
    void ResizeWindow()
    {
        vkDeviceWaitIdle(VkDeviceManager::GetDevice());
        int width = 0, height = 0;
        glfwGetFramebufferSize(windowHandler.p_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(windowHandler.p_window, &width, &height);
            glfwWaitEvents();
        }
        //we delete everything that was in the swapchain queue
        swapDeletionQueue.Flush();

        //then we recreate and move deletion functions back to swapchain queue
        VkSwapChainManager::InitSwapchain();
        swapDeletionQueue.PushFunction([=]() {
            VkSwapChainManager::DeleteSwapchain();
        });
        CreateMainFramebuffer();   
    }

    void UpdateDraw(float clearColor[4],std::function<void()>&& drawCalls)
    {
        VkCommandbufferManager::BeginCommands(cmd, swapchainImageIndex, [=] {ResizeWindow();});
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

        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        drawCalls();
        EndRenderpass();
        VkCommandbufferManager::EndCommands([=] {ResizeWindow();});
    }

    void CleanUpVulkan()
    {
        vkDeviceWaitIdle(VkDeviceManager::GetDevice());
        swapDeletionQueue.Flush();
        mainDeletionQueue.Flush();
        
    }

    void CreateMainFramebuffer()
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

    void CreateDefaultRenderpass()
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

    void CreateDescriptorSetLayoutBinding(DescripotrSetLayoutBindingInfo layoutBindingInfo)
    {
        descriptorSetLayoutBindings.push_back(vkinit::DescriptorsetLayoutBinding(layoutBindingInfo.descriptorType,layoutBindingInfo.shaderStageFlags,layoutBindingInfo.binding, layoutBindingInfo.descriptorCount));
    }

    void CreateDescriptorSetLayout(const std::string& layoutName)
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

    void RemoveDescriptorSetLayout(const std::string& layoutName)
    {
        vkDestroyDescriptorSetLayout(VkDeviceManager::GetDevice(), FindUnorderdMap(layoutName, descriptorSetLayout)->layout, nullptr);
    }
    
    void CreateDescriptorSet(const std::string& descriptorName, const std::string& layoutName,const VkBufferCreateFlags& bufferUsage,AllocatedBuffer& allocatedBuffer, const size_t& dataSize, size_t byteOffset /*= 0*/, const bool& withFrameOverlap /*= false*/)
    {
        VkDescriptorBufferInfo BufferInfo = CreateDescriptorBuffer(allocatedBuffer, dataSize, bufferUsage, byteOffset);
        auto& bindings = FindUnorderdMap(layoutName, descriptorSetLayout)->bindings;
        if(withFrameOverlap == true)
        {
            for(int i = 0; i < FRAME_OVERLAP; i++)
            {
                for(int i = 0; i < bindings.size(); i++)
                {
                    vkcomponent::DescriptorBuilder::Begin(&descriptorLayoutCache, &descriptorAllocator)
                    .BindBuffer(bindings[i].binding, &BufferInfo, bindings[i].descriptorType, bindings[i].stageFlags)
                    .Build(VkCommandbufferManager::frames[i].descriptorSets[descriptorName]);
                }
            }
        }
        else
        {   
            for(int i = 0; i < bindings.size(); i++)
            {
                vkcomponent::DescriptorBuilder::Begin(&descriptorLayoutCache, &descriptorAllocator)
                .BindBuffer(bindings[i].binding, &BufferInfo, bindings[i].descriptorType, bindings[i].stageFlags)
                .Build(descriptorSets[descriptorName]);
            }
        }
    }

    void RemoveAllocatedBuffer(AllocatedBuffer& allocatedBuffer)
    {
        vkDeviceWaitIdle(VkDeviceManager::GetDevice());
        vmaDestroyBuffer(VkDeviceManager::GetAllocator(), allocatedBuffer.buffer, allocatedBuffer.allocation);
    }


    void CreateGraphicsPipeline(std::vector<std::string>& shaderPaths, const std::string& shaderName, const std::vector<std::string>& layoutNames, const VkRenderPass& renderPass /*= VK_NULL_HANDLE*/, const ShaderDescriptions* descriptions /*= nullptr*/)
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
        pipelineBuilder.rasterizer = vkinit::RasterizationStateCreateInfo(descriptions->polygonMode, descriptions->cullMode);

        //we dont use multisampling, so just run the default one
        pipelineBuilder.multisampling = vkinit::MultisamplingStateCreateInfo();

        //a single blend attachment with no blending and writing to RGBA
        if(descriptions->colorBlending)
        {
            pipelineBuilder.colorBlendAttachment = vkinit::ColorBlendAttachmentState();
        }

        //default depthtesting
        pipelineBuilder.depthStencil = vkinit::DepthStencilCreateInfo(descriptions->depthTesting, descriptions->depthTesting, descriptions->depthCompareType);

        //vertex input controls how to read vertices from vertex buffers. We arent using it yet
        pipelineBuilder.vertexInputInfo = vkinit::VertexInputStateCreateInfo();

        //input assembly is the configuration for drawing triangle lists, strips, or individual points.
        //we are just going to draw triangle list
        pipelineBuilder.inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        VertexInputDescription vertexDescription = VkVertex::GetVertexDescription(descriptions->vertexLocations);

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

    void BindGraphicsPipeline(const std::string& shaderName)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(shaderName, shaderProgram)->pass.pipeline);
        
    }
    void BindDescriptorSet(const std::string& descriptorName, const std::string& shaderName, const uint32_t& set /*= 0*/, const bool& withFrameOverlap /*= false*/)
    {
        if(withFrameOverlap == true)
        {
            auto& currentFrame = VkCommandbufferManager::GetCurrentFrame();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(shaderName, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, currentFrame.descriptorSets), 0, 0);
        }
        else
        {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,FindUnorderdMap(shaderName, shaderProgram)->pass.layout, set, 1, FindUnorderdMap(descriptorName, descriptorSets), 0, 0);
        }
    }

    void BindIndexBuffer(AllocatedBuffer& indexBuffer)
    {
        VkDeviceSize offset = 0;
        vkCmdBindIndexBuffer(cmd, indexBuffer.buffer,offset, VK_INDEX_TYPE_UINT32);
    }
    void BindVertexBuffer(AllocatedBuffer& vertexBuffer)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer.buffer, &offset);
    }

    void DrawIndexed(std::vector<std::uint32_t>& indices)
    {   
        vkCmdDrawIndexed(cmd, indices.size(), 1,0,0,0);
    }

    void BeginRenderpass(const float clearColor[4], const VkRenderPass& renderPass /*= VK_NULL_HANDLE*/)
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
            rpInfo = vkinit::RenderpassBeginInfo(renderPass, VkSwapChainManager::GetSwapchainExtent(), mainFramebuffer[swapchainImageIndex]);
        }
        else
        {
            rpInfo = vkinit::RenderpassBeginInfo(mainPass, VkSwapChainManager::GetSwapchainExtent(), mainFramebuffer[swapchainImageIndex]);
        }
        //connect clear values
        rpInfo.clearValueCount = 2;

        VkClearValue clearValues[] = { clearValue, depthClear };

        rpInfo.pClearValues = &clearValues[0];
        
        vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        
    }
    void EndRenderpass()
    {
        //finalize the render pass
        vkCmdEndRenderPass(cmd);
    }
}
