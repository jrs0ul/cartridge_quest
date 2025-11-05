#include "RenderTexture.h"
#include "Extensions.h"
#include "SDLVideo.h"


void RenderTexture::create(uint32_t width,
                           uint32_t height,
                           uint8_t filter,
                           bool isVulkan,
                           VkDevice* device,
                           VkPhysicalDevice* physical,
                           size_t vkSwapChainImageCount
                           )
{
    useVulkan = isVulkan;
    _width = width;
    _height = height;

    printf("creating render texture %ux%u\n", _width, _height);

    if (!isVulkan)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == 1 ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == 1 ? GL_LINEAR : GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else  // VULKAN
    {
        SDLVideo::createImage(*device,
                              *physical,
                              _width,
                              _height,
                              VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                              vkImage,
                              vkTextureMemory);

        VkImageView vkImageView = SDLVideo::createImageView(*device,
                                                            vkImage,
                                                            VK_FORMAT_R8G8B8A8_SRGB,
                                                            VK_IMAGE_ASPECT_COLOR_BIT);


        VkAttachmentDescription colorAttachment{};
        colorAttachment.format         = VK_FORMAT_R8G8B8A8_SRGB;
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments    = &colorAttachment;
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;

        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_NONE_KHR;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        vkCreateRenderPass(*device, &renderPassInfo, nullptr, &vkRenderPass);
        //---
        vkSwapChainFramebuffers.resize(vkSwapChainImageCount);

        for (size_t i = 0; i < vkSwapChainImageCount; i++)
        {
            VkImageView attachments[] = {vkImageView};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = vkRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = _width;
            framebufferInfo.height          = _height;
            framebufferInfo.layers          = 1;

            vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &vkSwapChainFramebuffers[i]);
        }
        //----
        vkFences.resize(vkSwapChainImageCount);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &vkImageAvailableSemaphore);
        vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &vkRenderingFinishedSemaphore);

        for (size_t i = 0; i < vkSwapChainImageCount; i++)
        {
            vkCreateFence(*device, &fenceInfo, nullptr, &vkFences[i]);
        }


    }
}

void RenderTexture::bind()
{
    if (!useVulkan)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, _width, _height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else //VULKAN
    {
        /*vkAcquireNextImageKHR(*device,
                          vkSwapchain,
                          UINT64_MAX,
                          vkImageAvailableSemaphore,
                          VK_NULL_HANDLE,
                          &vkFrameIndex);

        vkWaitForFences(vkDevice, 1, &vkFences[vkFrameIndex], VK_FALSE, UINT64_MAX);
        vkResetFences(vkDevice, 1, &vkFences[vkFrameIndex]);

        vkCommandBuffer = vkCommandBuffers[vkFrameIndex];
        vkImage = vkSwapchainImages[vkFrameIndex];

        vkResetCommandBuffer(vkCommandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass        = vkRenderPass;
        render_pass_info.framebuffer       = vkSwapchainFramebuffers[vkFrameIndex];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = vkSwapchainSize;

        std::vector<VkClearValue> clearValues(1);
        clearValues[0].color = {0,0,0,0};

        render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
        render_pass_info.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(vkCommandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);*/
    }

}

void RenderTexture::unbind()
{
    if (!useVulkan)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else //VULKAN
    {
       /* vkCmdEndRenderPass(*vkCommandBuffer);
        vkEndCommandBuffer(*vkCommandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = vkCommandBuffer;
        vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkFences[vkFrameIndex]);

        vkDeviceWaitIdle(*device);*/


    }
}


void RenderTexture::destroy()
{
    if (!useVulkan)
    {
        glDeleteFramebuffers(1, &fbo);
    }
}
