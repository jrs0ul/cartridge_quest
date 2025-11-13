/*
 The Disarray
 by jrs0ul(jrs0ul ^at^ gmail ^dot^ com) 2025
 -------------------------------------------
 Sprite batcher
 */

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cwchar>
#ifndef __ANDROID__
#include "Extensions.h"
#endif
#include "SpriteBatcher.h"
#include "Vectors.h"
#include "OSTools.h"
#include "Xml.h"
#include "SDLVideo.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#endif




GLuint SpriteBatcher::getGLName(unsigned long index)
{
    if (index < glTextures.size())
    {
        return glTextures[index];
    }

    return 0;
}
//-----------------------------

PicData* SpriteBatcher::getInfo(unsigned long index)
{
    const uint32_t texCount = (isVulkan) ? vkTextures.size() : glTextures.size();

    if (index < texCount)
    {
        return &picInfo[index];
    }

    return 0;
}



VkCommandBuffer beginSingleTimeCommands(VkDevice& device, VkCommandPool& commandPool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkDevice& device,
                           VkCommandPool& commandPool,
                           VkQueue& graphicsQueue,
                           VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void copyBufferToImage(VkDevice& device, VkCommandPool& commandPool, VkQueue& graphicsQueue,
                       VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
}


void transitionImageLayout(VkDevice& device,
                           VkCommandPool& commandPool,
                           VkQueue& graphicsQueue,
                           VkImage image,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

    endSingleTimeCommands(device, commandPool, graphicsQueue, commandBuffer);
}


//-----------------------------
#ifndef __ANDROID__
bool SpriteBatcher::load(const char* list, 
                         bool useVulkan,
                         VkDevice* vkDevice,
                         VkPhysicalDevice* physical,
                         VkCommandPool* vkCommandPool,
                         VkQueue* vkGraphicsQueue
                         )
#else
bool SpriteBatcher::load(const char* list, AAssetManager* assman, 
                         bool useVulkan,
                         VkDevice* vkDevice,
                         VkPhysicalDevice* physical,
                         VkCommandPool* vkCommandPool,
                         VkQueue* vkGraphicsQueue
                         )
#endif
{

#ifndef __ANDROID__
    if (!initContainer(list, useVulkan))
#else
    if (!initContainer(list, assman, useVulkan))
#endif
    {
        return false;
    }

    for (unsigned long i = 0; i < picInfo.size(); ++i)
    {

        Image newImg;
        unsigned short imageBits = 0;

#ifdef __ANDROID__
        if (!newImg.loadTga(PicInfo[i].name, imageBits, assman))
        {
            LOGI("%s not found or corrupted by M$\n", picInfo[i].name);
        }
#else
        if (!newImg.loadTga(picInfo[i].name, imageBits))
        {
            printf("%s not found or corrupted by M$\n", picInfo[i].name);
        }
#endif

        picInfo[i].width  = newImg.width;
        picInfo[i].height = newImg.height;


        picInfo[i].htilew  = picInfo[i].twidth / 2.0f;
        picInfo[i].htileh  = picInfo[i].theight / 2.0f;
        picInfo[i].vframes = picInfo[i].height / picInfo[i].theight;
        picInfo[i].hframes = picInfo[i].width / picInfo[i].twidth;


        if (!useVulkan)
        {
            int filter = GL_NEAREST;

            if (picInfo[i].filter)
            {
                filter = GL_LINEAR;
            }

            glBindTexture(GL_TEXTURE_2D, glTextures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );

            if (imageBits > 24)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             newImg.width, newImg.height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             newImg.data);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                             newImg.width, newImg.height,
                             0, GL_RGB, GL_UNSIGNED_BYTE,
                             newImg.data);
            }
        }
        else //VULKAN
        {

            VkDeviceSize imageSize = newImg.width * newImg.height * (newImg.bits / 8);
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            SDLVideo::createBuffer(*vkDevice,
                                   *physical,
                                   imageSize,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingBuffer,
                                   stagingBufferMemory);

            void* data;
            vkMapMemory(*vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, newImg.data, static_cast<size_t>(imageSize));
            vkUnmapMemory(*vkDevice, stagingBufferMemory);


            VulkanTexture t;

            SDLVideo::createImage(*vkDevice,
                                  *physical,
                                  static_cast<uint32_t>(newImg.width),
                                  static_cast<uint32_t>(newImg.height),
                                  newImg.bits > 24 ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8_SRGB,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                  t.vkImage,
                                  t.vkTextureMemory);

            transitionImageLayout(*vkDevice,
                                  *vkCommandPool,
                                  *vkGraphicsQueue,
                                  t.vkImage,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            copyBufferToImage(*vkDevice,
                              *vkCommandPool,
                              *vkGraphicsQueue,
                              stagingBuffer,
                              t.vkImage,
                              static_cast<uint32_t>(newImg.width),
                              static_cast<uint32_t>(newImg.height));

            transitionImageLayout(*vkDevice,
                                  *vkCommandPool,
                                  *vkGraphicsQueue,
                                  t.vkImage,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vkDestroyBuffer(*vkDevice, stagingBuffer, nullptr);
            vkFreeMemory(*vkDevice, stagingBufferMemory, nullptr);


            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = t.vkImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = newImg.bits > 24 ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8_SRGB;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;


            if (vkCreateImageView(*vkDevice, &viewInfo, nullptr, &t.vkImageView) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture image view!");
            }

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

            VkFilter filter = VK_FILTER_NEAREST;

            if (picInfo[i].filter)
            {
                filter = VK_FILTER_LINEAR;
            }

            samplerInfo.magFilter = filter;
            samplerInfo.minFilter = filter;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            if (vkCreateSampler(*vkDevice, &samplerInfo, nullptr, &t.vkSampler) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create texture sampler!");
            }


            vkTextures.push_back(t);

        }

        newImg.destroy();

    } // for

    return true;
}
//--------------------------------------------------
void SpriteBatcher::bindTexture(unsigned long index,
                                ShaderProgram* shader,
                                bool useVulkan,
                                VkDevice* vkDevice)
{
    if (useVulkan)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vkTextures[index].vkImageView;
        imageInfo.sampler = vkTextures[index].vkSampler;

        VkWriteDescriptorSet ds{};
        ds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ds.dstSet = shader->vkDS;
        ds.dstBinding = 0;
        ds.dstArrayElement = 0;
        ds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        ds.descriptorCount = 1;
        ds.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(*vkDevice, 1, &ds, 0, nullptr);

    }
}

//---------------------------------------------------
void SpriteBatcher::draw(
                long textureIndex,
                float x, float y,
                unsigned int frame,
                bool useCenter,
                float scaleX, float scaleY,
                float rotationAngle,
                COLOR upColor,
                COLOR dwColor,
                bool flipColors
               )
{

    SpriteBatchItem nb;

    nb.x = x;
    nb.y = y;
    nb.textureIndex = textureIndex;
    nb.frame = frame;
    nb.useCenter = useCenter;
    nb.scaleX = scaleX;
    nb.scaleY = scaleY;
    nb.rotationAngle = rotationAngle;

    if (!flipColors)
    {
        nb.upColor[0] = upColor;
        nb.upColor[1] = upColor;
        nb.dwColor[0] = dwColor;
        nb.dwColor[1] = dwColor;
    }
    else
    {
        nb.upColor[0] = upColor;
        nb.dwColor[0] = upColor;
        nb.upColor[1] = dwColor;
        nb.dwColor[1] = dwColor;
    }
        batch.push_back(nb);
}
//---------------------------------------------------
void SpriteBatcher::drawVA(void * vertices,
                           void * uvs,
                           void *colors,
                           unsigned uvsCount,
                           unsigned vertexCount,
                           ShaderProgram* shader,
                           bool useVulkan,
                           VkCommandBuffer* vkCmd)
{
    int attribID = 0;
    int ColorAttribID = 0;
    int UvsAttribID = 0;

    if (!useVulkan)
    {
        attribID = shader->getAttributeID("aPosition"); 
        ColorAttribID = shader->getAttributeID("aColor");
        glVertexAttribPointer(attribID, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(attribID);

        if (uvsCount)
        {
            UvsAttribID = shader->getAttributeID("aUvs");
            glVertexAttribPointer(UvsAttribID, 2, GL_FLOAT, GL_FALSE, 0, uvs);
            glEnableVertexAttribArray(UvsAttribID);
        }

        glVertexAttribPointer(ColorAttribID, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glEnableVertexAttribArray(ColorAttribID);

        glDrawArrays(GL_TRIANGLES, 0, vertexCount / 2 );

        glDisableVertexAttribArray(ColorAttribID);

        if (uvsCount)
        {
            glDisableVertexAttribArray(UvsAttribID);
        }

        glDisableVertexAttribArray(attribID);
    }
    else // VULKAN
    {

        memcpy(&((float*)(shader->vkMappedBuffer[0]))[shader->vkBufferOffset[0] / sizeof(float)], vertices, vertexCount * sizeof(float));

        if (uvsCount)
        {
            memcpy(&((float*)(shader->vkMappedBuffer[1]))[shader->vkBufferOffset[1] / sizeof(float)], uvs, sizeof(float) * vertexCount);
        }

        memcpy(&((float*)(shader->vkMappedBuffer[2]))[shader->vkBufferOffset[2] / sizeof(float)], colors, sizeof(float) * vertexCount * 2);

        vkCmdBindVertexBuffers(*vkCmd, 0, 1, &shader->vkVertexBuffers[0], &shader->vkBufferOffset[0]);

        if (uvsCount)
        {
            vkCmdBindVertexBuffers(*vkCmd, 1, 1, &shader->vkVertexBuffers[1], &shader->vkBufferOffset[1]);
        }

        vkCmdBindVertexBuffers(*vkCmd, 2, 1, &shader->vkVertexBuffers[2], &shader->vkBufferOffset[2]);

        vkCmdBindDescriptorSets(*vkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->vkPipelineLayout, 0, 1, &shader->vkDS, 0, nullptr);

        vkCmdDraw(*vkCmd, vertexCount / 2, 1, 0, 0);

        if (uvsCount)
        {
            shader->vkBufferOffset[1] += vertexCount * sizeof(float);
        }

        shader->vkBufferOffset[2] += (vertexCount * sizeof(float)*2);
        shader->vkBufferOffset[0] += vertexCount * sizeof(float);

    }
}
//----------------------------------------------------------
Vector3D CalcUvs(PicData * p, unsigned frame)
{

    //printf("%d\n", p->hframes);
    float hf = frame / p->hframes;

    float startx =  (frame - p->hframes * hf) * p->twidth;
    float starty = hf * p->theight;

    Vector3D result = Vector3D(
                               (startx * 1.0f) / (p->width * 1.0f),
                               ((startx + p->twidth) * 1.0f) / (p->width * 1.0f),
                               (( p->height - starty) * 1.0f ) / ( p->height * 1.0f ),//- 0.0001f,
                               (( p->height - starty - p->theight ) * 1.0f) / (p->height * 1.0f)
                              );
    return result;
}

//----------------------------------------------------------
void SpriteBatcher::drawBatch(ShaderProgram *  justColor,
                              ShaderProgram *  uvColor,
                              int              method,
                              bool             useVulkan,
                              VkCommandBuffer* vkCmd,
                              VkDevice*        vkDevice)
{

        for (int i = 0; i < ShaderProgram::VULKAN_BUFFER_COUNT; ++i)
        {
            justColor->vkBufferOffset[i] = 0;
            uvColor->vkBufferOffset[i] = 0;
        }

        switch(method){
              //TODO: complete VA
        default:{
                std::vector<float> vertices;
                std::vector<float> uvs;
                float * colors = 0;
                uint32_t colorIndex = 0;

                if (batch.size())
                {
                    colors = (float*)malloc(sizeof(float) * 24 * batch.size());
                }

                Vector3D uv(0.0f, 0.0f, 0.0f, 0.0f);
                float htilew, htileh;
                float twidth, theight;


                long texIndex = -1;

                for (unsigned long i = 0; i < batch.size(); ++i)
                {
                    PicData * p = 0;

                    htilew = 0.5f; htileh = 0.5f;
                    twidth = 1; theight = 1;

                    if ((batch[i].textureIndex >= 0) && ( batch[i].textureIndex < (long)count()))
                    {
                        p = &picInfo[batch[i].textureIndex];
                    }

                    if (p)
                    {
                        if (p->sprites.size()) //iregular sprites in atlas
                        {
                            Sprite* spr = &(p->sprites[batch[i].frame]);

                            float startX = spr->startX / (p->width * 1.f);
                            float endX   = (spr->startX + spr->width) / (p->width * 1.f);
                            float startY =  ((p->height - spr->startY - spr->height) * 1.f) / (p->height * 1.f);
                            float endY = ((p->height - spr->startY) * 1.f) / (p->height * 1.f);

                            uv = Vector3D(startX, endX, endY, startY);
                            htilew = spr->width / 2.0f; 
                            htileh = spr->height / 2.0f;
                            twidth = spr->width; 
                            theight = spr->height;
                        }
                        else //all sprites of same size
                        {
                            uv = CalcUvs(p, batch[i].frame);
                            htilew = p->twidth / 2.0f; 
                            htileh = p->theight / 2.0f;
                            twidth = p->twidth; 
                            theight = p->theight;
                        }
                    }


                    ShaderProgram* currentShader = uvColor;

                    if ((texIndex != batch[i].textureIndex)
                        && (vertices.size() > 0))
                    {
                        //let's draw old stuff
                        if ((texIndex >= 0) && (texIndex < (long)count()))
                        {

                            if (!useVulkan)
                            {
                                glBindTexture(GL_TEXTURE_2D, glTextures[texIndex]);
                            }


                            if (uvColor)
                            {
                                uvColor->use(vkCmd);

                                if (useVulkan)
                                {
                                    bindTexture(texIndex, uvColor, true, vkDevice);
                                }

                            }
                        }

                        else{

                            if (!useVulkan)
                            {
                                glBindTexture(GL_TEXTURE_2D, 0);
                            }

                            if (justColor)
                            {
                                justColor->use(vkCmd);
                                currentShader = justColor;
                                uvs.clear();
                            }
                        }

                        drawVA(vertices.data(), uvs.data(), colors,
                               uvs.size(), vertices.size(), currentShader, useVulkan, vkCmd);

                        vertices.clear();
                        uvs.clear();
                        colorIndex = 0;

                }

                texIndex = batch[i].textureIndex;

                //append to arrays
                uvs.push_back(uv.v[0]); uvs.push_back(uv.v[2]);
                uvs.push_back(uv.v[1]); uvs.push_back(uv.v[2]);
                uvs.push_back(uv.v[1]); uvs.push_back(uv.v[3]);
                uvs.push_back(uv.v[0]); uvs.push_back(uv.v[2]);
                uvs.push_back(uv.v[1]); uvs.push_back(uv.v[3]);
                uvs.push_back(uv.v[0]); uvs.push_back(uv.v[3]); 

                const size_t COLOR_DATA_SIZE = sizeof(float) * 4;
                void* triangleColors[] = {batch[i].upColor[0].c, batch[i].upColor[1].c,
                                          batch[i].dwColor[1].c, batch[i].upColor[0].c,
                                          batch[i].dwColor[1].c, batch[i].dwColor[0].c};

                for (int j = 0; j < 6; ++j)
                {
                    memcpy(&colors[colorIndex], triangleColors[j], COLOR_DATA_SIZE);
                    colorIndex += 4;
                }

                //---
                if (batch[i].rotationAngle == 0.0f)
                {
                    if (batch[i].useCenter)
                    {
                        float hwidth = htilew * batch[i].scaleX;
                        float hheight = htileh * batch[i].scaleY;


                        vertices.push_back(batch[i].x - hwidth); 
                        vertices.push_back(batch[i].y - hheight);

                        vertices.push_back(batch[i].x + hwidth); 
                        vertices.push_back(batch[i].y - hheight);

                        vertices.push_back(batch[i].x + hwidth); 
                        vertices.push_back(batch[i].y + hheight);

                        vertices.push_back(batch[i].x - hwidth); 
                        vertices.push_back(batch[i].y - hheight);

                        vertices.push_back(batch[i].x + hwidth); 
                        vertices.push_back(batch[i].y + hheight);

                        vertices.push_back(batch[i].x - hwidth); 
                        vertices.push_back(batch[i].y + hheight);
                    }
                    else
                    {

                        vertices.push_back(batch[i].x);
                        vertices.push_back(batch[i].y);

                        vertices.push_back(batch[i].x + twidth * batch[i].scaleX);
                        vertices.push_back(batch[i].y);

                        vertices.push_back(batch[i].x + twidth * batch[i].scaleX); 
                        vertices.push_back(batch[i].y + theight * batch[i].scaleY);

                        vertices.push_back(batch[i].x);
                        vertices.push_back(batch[i].y);

                        vertices.push_back(batch[i].x + twidth * batch[i].scaleX); 
                        vertices.push_back(batch[i].y + theight * batch[i].scaleY);


                        vertices.push_back(batch[i].x); 
                        vertices.push_back(batch[i].y + theight * batch[i].scaleY);
                    }
                }
                else{

                //TODO: non-centered rotation

                    float angle = batch[i].rotationAngle * 0.0174532925 + 3.14f;

                    if (batch[i].useCenter)
                    {
                        float hwidth = htilew * batch[i].scaleX;
                        float hheight = htileh * batch[i].scaleY;

                        float co = cosf(angle);
                        float si = sinf(angle);
                        float cos_rot_w = co * hwidth;
                        float cos_rot_h = co * hheight;
                        float sin_rot_w = si * hwidth;
                        float sin_rot_h = si * hheight;


                        vertices.push_back(batch[i].x + cos_rot_w - sin_rot_h); 
                        vertices.push_back(batch[i].y + sin_rot_w + cos_rot_h);

                        vertices.push_back(batch[i].x - cos_rot_w - sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w + cos_rot_h);

                        vertices.push_back(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.push_back(batch[i].x + cos_rot_w - sin_rot_h); 
                        vertices.push_back(batch[i].y + sin_rot_w + cos_rot_h);

                        vertices.push_back(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.push_back(batch[i].x + cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y + sin_rot_w - cos_rot_h);
                    }
                    else{

                        float _width = twidth * batch[i].scaleX;
                        float _height = theight * batch[i].scaleY;

                        float co = cosf(angle);
                        float si = sinf(angle);
                        float cos_rot_w = co * _width;
                        float cos_rot_h = co * _height;
                        float sin_rot_w = si * _width;
                        float sin_rot_h = si * _height;

                        //TODO: fix this

                        vertices.push_back(batch[i].x); 
                        vertices.push_back(batch[i].y);

                        vertices.push_back(batch[i].x - cos_rot_w - sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w + cos_rot_h);

                        vertices.push_back(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.push_back(batch[i].x); 
                        vertices.push_back(batch[i].y);

                        vertices.push_back(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.push_back(batch[i].x + cos_rot_w + sin_rot_h); 
                        vertices.push_back(batch[i].y + sin_rot_w - cos_rot_h);

                    }

                }
            } //for

            if (vertices.size() > 0){

                ShaderProgram* currentShader = uvColor;

                if ((texIndex >= 0) && (texIndex < (long)count()))
                {

                    if (!useVulkan)
                    {
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, glTextures[texIndex]);
                    }

                    if (uvColor)
                    {
                        uvColor->use(vkCmd);

                        if (useVulkan)
                        {
                            bindTexture(texIndex, uvColor, true, vkDevice);
                        }
                    }
                }
                else
                {
                    if (!useVulkan)
                    {
                        glBindTexture(GL_TEXTURE_2D, 0);
                        glDisable(GL_TEXTURE_2D);
                    }

                    if (justColor)
                    {
                        justColor->use(vkCmd);
                        currentShader = justColor;
                        uvs.clear();
                    }
                }

                drawVA(vertices.data(), uvs.data(), colors,
                       uvs.size(), vertices.size(), currentShader, useVulkan, vkCmd);


                vertices.clear();
                uvs.clear();
                colorIndex = 0;
            }

            if (colors)
            {
                free(colors);
            }
        }
    }

    batch.clear();
}
//-----------------------------------------------------
void SpriteBatcher::resizeContainer(unsigned long index,
                                    int twidth, int theight, int filter,
                                    const char * name,
                                    bool createTextures,
                                    GLuint texname)
{

    if (picInfo.size() < index + 1)
    {

        GLuint glui = texname;
        PicData p;
        p.twidth = twidth;
        p.theight = theight;
        p.filter = filter;

        for (unsigned i = picInfo.size(); i < index + 1; i++)
        {
            picInfo.push_back(p);
            glTextures.push_back(glui);
        }

        if (createTextures)
        {
            glGenTextures(1, ((GLuint *)glTextures.data()) + index);
        }

        char * copy = (char*)malloc(strlen(name)+1);
        strcpy(copy, name);
        char * res = 0;
        res = strtok(copy, "/");

        while (res)
        {
            strcpy(picInfo[index].name, res);
            res = strtok(0, "/");
        }

        free(copy);

    }
    else
    {
        PicData * pp = &picInfo[index];
        pp->twidth = twidth;
        pp->theight = theight;
        pp->filter = filter;
        char * copy = (char*)malloc(strlen(name)+1);
        strcpy(copy, name);
        char * res = 0;
        res = strtok(copy, "/");

        while (res)
        {
            strcpy(pp->name, res);
            res = strtok(0, "/");
        }

        free(copy);

        if (glIsTexture(glTextures[index]))
        {
            glDeleteTextures(1, ((GLuint *)glTextures.data()) + index);
        }

        if (createTextures)
        {
            glGenTextures(1, ((GLuint *)glTextures.data()) + index);
        }
        else
        {
            *(((GLuint *)glTextures.data()) + index) = texname;
        }

    }


}

//-----------------------------------------------------
#ifndef __ANDROID__
bool SpriteBatcher::loadFile(const char* file,
                             unsigned long index,
                             int twidth,
                             int theight,
                             int filter)
#else
bool SpriteBatcher::loadFile(const char* file,
                             unsigned long index,
                             int twidth,
                             int theight,
                             int filter,
                             AAssetManager* man)
#endif
{

        Image naujas;

        unsigned short imageBits=0;


#ifdef __ANDROID__
        if (!naujas.loadTga(file, imageBits, man)){
            LOGI("%s not found or corrupted by M$\n", file);
#else

        if (!naujas.loadTga(file, imageBits)){
            printf("%s not found or corrupted by M$\n", file);
#endif
            return false;
        }


        resizeContainer(index, twidth, theight, filter, file);

        picInfo[index].width = naujas.width;
        picInfo[index].height = naujas.height;


        picInfo[index].htilew  = picInfo[index].twidth / 2.0f;
        picInfo[index].htileh  = picInfo[index].theight / 2.0f;
        picInfo[index].vframes = picInfo[index].height / picInfo[index].theight;
        picInfo[index].hframes = picInfo[index].width / picInfo[index].twidth;

        int filtras = GL_NEAREST;

        if (picInfo[index].filter)
        {
            filtras = GL_LINEAR;
        }


        glBindTexture(GL_TEXTURE_2D, glTextures[index]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtras );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtras );

        GLint border = 0;

        if (imageBits > 24)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, naujas.width, naujas.height,
                 border, GL_RGBA, GL_UNSIGNED_BYTE,naujas.data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, naujas.width, naujas.height,
                 border, GL_RGB, GL_UNSIGNED_BYTE,naujas.data);

        naujas.destroy();

        return true;

}
//---------------------------------------------------
void SpriteBatcher::makeTexture(Image& img,
                                const char * name,
                                unsigned long index,
                                int twidth,
                                int theight,
                                int filter)
{

    resizeContainer(index, twidth, theight, filter, name);

    picInfo[index].width = img.width;
    picInfo[index].height = img.height;


    picInfo[index].htilew  = picInfo[index].twidth / 2.0f;
    picInfo[index].htileh  = picInfo[index].theight / 2.0f;
    picInfo[index].vframes = picInfo[index].height / picInfo[index].theight;
    picInfo[index].hframes = picInfo[index].width / picInfo[index].twidth;

    int filtras = GL_NEAREST;

    if (picInfo[index].filter)
    {
        filtras = GL_LINEAR;
    }


    glBindTexture(GL_TEXTURE_2D, glTextures[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtras );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtras );

    GLint border = 0;

    if (img.bits > 24)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
                border, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height,
                border, GL_RGB, GL_UNSIGNED_BYTE, img.data);
    }

}

//--------------------------------------------------
bool SpriteBatcher::loadFile(unsigned long index,
                             const char * BasePath)
{

    Image naujas;

    unsigned short imageBits = 0;


    char dir[512];
    char buf[1024];

    sprintf(dir, "%spics/", BasePath);
    sprintf(buf, "%s%s", dir, picInfo[index].name);

#ifndef __ANDROID__
    if (!naujas.loadTga(buf, imageBits)){
#else
    if (!naujas.loadTga(buf, imageBits, 0)){
#endif

        sprintf(buf, "base/pics/%s", picInfo[index].name);
        puts("Let's try base/");
#ifndef __ANDROID__
        if (!naujas.loadTga(buf, imageBits)){
#else
        if (!naujas.loadTga(buf, imageBits, 0)){
#endif
            printf("%s not found or corrupted by M$\n", buf);
            return false;
        }

    }

    picInfo[index].width = naujas.width;
    picInfo[index].height = naujas.height;


    picInfo[index].htilew  = picInfo[index].twidth / 2.0f;
    picInfo[index].htileh  = picInfo[index].theight / 2.0f;
    picInfo[index].vframes = picInfo[index].height / picInfo[index].theight;
    picInfo[index].hframes = picInfo[index].width / picInfo[index].twidth;

    int filtras = GL_NEAREST;

    if (picInfo[index].filter)
    {
        filtras = GL_LINEAR;
    }


    glBindTexture(GL_TEXTURE_2D, glTextures[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtras );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtras );

    if (imageBits > 24)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, naujas.width, naujas.height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE,naujas.data);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, naujas.width, naujas.height,
                     0, GL_RGB, GL_UNSIGNED_BYTE,naujas.data);
    }

    naujas.destroy();

    return true;

}
//---------------------------------
void SpriteBatcher::attachTexture(GLuint textureID, unsigned long index,
                                  int width, int height,
                                  int twidth, int theight, int filter)
{

    resizeContainer(index, twidth, theight, filter, "lol", false, textureID);
    picInfo[index].width = width;
    picInfo[index].height = height;


    picInfo[index].htilew = picInfo[index].twidth / 2.0f;
    picInfo[index].htileh = picInfo[index].theight / 2.0f;
    picInfo[index].vframes = picInfo[index].height / picInfo[index].theight;
    picInfo[index].hframes = picInfo[index].width / picInfo[index].twidth;


    int filtras = GL_NEAREST;

    if (picInfo[index].filter)
    {
        filtras = GL_LINEAR;
    }

    glBindTexture(GL_TEXTURE_2D, glTextures[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtras );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtras );
    glBindTexture(GL_TEXTURE_2D, 0);

}

//---------------------------
void SpriteBatcher::attachTexture(VulkanTexture& tex, unsigned long index,
                                  int width, int height,
                                  int twidth, int theight,
                                  VkDevice* device, int filter)
{

    PicData newData;

    newData.twidth  = twidth;
    newData.theight = theight;
    newData.filter  = filter;
    newData.width   = width;
    newData.height  = height;

    newData.htilew = newData.twidth / 2.0f;
    newData.htileh = newData.theight / 2.0f;
    newData.vframes = newData.height / newData.theight;
    newData.hframes = newData.width / newData.twidth;

    picInfo.push_back(newData);


    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    VkFilter vkfilter = VK_FILTER_NEAREST;

    if (picInfo[index].filter)
    {
        vkfilter = VK_FILTER_LINEAR;
    }

    samplerInfo.magFilter = vkfilter;
    samplerInfo.minFilter = vkfilter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (vkCreateSampler(*device, &samplerInfo, nullptr, &tex.vkSampler) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create texture sampler!");
    }


    vkTextures.push_back(tex);
}


//--------------------------------
int SpriteBatcher::findByName(const char* picname, bool debug)
{
    unsigned long start = 0;

    if (!picInfo.size())
    {
        return -1;
    }

    while ((strcmp(picInfo[start].name, picname) != 0) && (start < picInfo.size()))
    {
        if (debug)
        {
            puts(picInfo[start].name);
        }
        start++;
    }

    if (start == picInfo.size())
    {
        return -1;
    }

    return start;
}
//---------------------------------------
#ifndef __ANDROID__
bool SpriteBatcher::initContainer(const char* list,
                                  bool useVulkan)
#else
bool SpriteBatcher::initContainer(const char* list,
                                  AAssetManager* assman,
                                  bool useVulkan)
#endif
{

    isVulkan = useVulkan;

    Xml pictureList;

#ifndef __ANDROID__
    bool result = pictureList.load(list);
#else


    bool result = pictureList.load(list, assman);

#endif

    if (result)
    {
        XmlNode *mainnode = pictureList.root.getNode(L"Images");

        int imageCount = 0;

        if (mainnode)
        {
            imageCount = mainnode->childrenCount();
        }

        for (int i = 0; i < imageCount; ++i)
        {
            PicData data;

            XmlNode *node = mainnode->getNode(i);

            if (node)
            {
                for (int j = 0; j < (int)node->attributeCount(); ++j)
                {
                    XmlAttribute *atr = node->getAttribute(j);

                    if (atr)
                    {
                        if (wcscmp(atr->getName(), L"src") == 0)
                        {
                            wchar_t* value = atr->getValue();

                            if (value)
                            {
                                sprintf(data.name, "%ls", value);
                            }
                        }
                        else if (wcscmp(atr->getName(), L"width") == 0)
                        {
                            char buffer[100];
                            wchar_t* value = atr->getValue();

                            if (value)
                            {
                                sprintf(buffer, "%ls", value);
                                data.twidth = atoi(buffer);
                            }
                        }
                        else if (wcscmp(atr->getName(), L"height") == 0)
                        {
                            char buffer[100];
                            wchar_t* value = atr->getValue();

                            if (value)
                            {
                                sprintf(buffer, "%ls", value);
                                data.theight = atoi(buffer);
                            }
                        }
                        else if (wcscmp(atr->getName(), L"filter") == 0)
                        {
                            char buffer[100];
                            wchar_t* value = atr->getValue();

                            if (value)
                            {
                                sprintf(buffer, "%ls", value);
                                data.filter = atoi(buffer);
                            }
                        }
                    }
                }

                XmlNode* pathNode = node->getNode(L"Path");

                if (pathNode)
                {
                    sprintf(data.name, "%ls", pathNode->getValue());
                }

                XmlNode* spritesNode = node->getNode(L"Sprites");

                if (spritesNode)
                {
                    for (int j = 0; j < (int)spritesNode->childrenCount(); ++j)
                    {
                        XmlNode* sprite = spritesNode->getNode(j);
                        if (sprite)
                        {
                            Sprite spr;

                            for (int h = 0; h < (int) sprite->attributeCount(); ++h)
                            {

                                XmlAttribute* at = sprite->getAttribute(h);

                                char buffer[100];
                                sprintf(buffer, "%ls", at->getName());

                                if (strcmp(buffer, "name") == 0)
                                {
                                    sprintf(spr.name, "%ls", at->getValue());
                                }
                                else if (strcmp(buffer, "x") == 0)
                                {
                                    sprintf(buffer, "%ls", at->getValue());
                                    spr.startX = atoi(buffer);
                                }
                                else if (strcmp(buffer, "y") == 0)
                                {
                                    sprintf(buffer, "%ls", at->getValue());
                                    spr.startY = atoi(buffer);
                                }
                                else if (strcmp(buffer, "width") == 0)
                                {
                                    sprintf(buffer, "%ls", at->getValue());
                                    spr.width = atoi(buffer);
                                }
                                else if (strcmp(buffer, "height") == 0)
                                {
                                    sprintf(buffer, "%ls", at->getValue());
                                    spr.height = atoi(buffer);
                                }

                            }

                            data.sprites.push_back(spr);

                        }
                    }

                }

            }

#ifdef __ANDROID__
//            LOGI(">>>Image: name:%s width:%d height:%d filter%d\n", data.name, data.twidth, data.theight, data.filter);
#else

//            printf(">>>Image: name:%s width:%d height:%d filter%d\n", data.name, data.twidth, data.theight, data.filter);
#endif
            picInfo.push_back(data);
        }

        if (!useVulkan)
        {
            for (unsigned long i = 0; i < picInfo.size(); i++) 
            {
                GLuint glui = 0;
                glTextures.push_back(glui);
            }

            printf("Creating %lu opengl textures\n", picInfo.size());
            glGenTextures(picInfo.size(), (GLuint *)glTextures.data());
        }
        else // VULKAN
        {
                    }

    }

    pictureList.destroy();

    return result;

}

//----------------------------------
void SpriteBatcher::destroy(VkDevice* vkDevice)
{
    if (!isVulkan)
    {

        for (unsigned long i = 0; i < glTextures.size(); i++)
        {
            if (glIsTexture(glTextures[i]))
            {
                glDeleteTextures(1, ((GLuint *)glTextures.data()) + i);
            }
        }

        glTextures.clear();
    }
    else // Vulkan
    {
        printf("Deleting Vulkan textures...\n");

        for (unsigned long i = 0; i < vkTextures.size(); ++i)
        {
            vkDestroySampler(*vkDevice, vkTextures[i].vkSampler, nullptr);
            vkDestroyImageView(*vkDevice, vkTextures[i].vkImageView, nullptr);
            vkDestroyImage(*vkDevice, vkTextures[i].vkImage, nullptr);
            vkFreeMemory(*vkDevice, vkTextures[i].vkTextureMemory, nullptr);
        }

        vkTextures.clear();

       
    }

    batch.clear();

    for (unsigned long i = 0; i < picInfo.size(); ++i)
    {
        picInfo[i].sprites.clear();
    }

    picInfo.clear();
}

//-------------------------------
void SpriteBatcher::remove(unsigned long index)
{
    if (index < glTextures.size())
    {
        if (glIsTexture(glTextures[index]))
        {
            glDeleteTextures(1, ((GLuint *)glTextures.data()) + index);
        }

        glTextures.erase(glTextures.begin() + index);
        picInfo.erase(picInfo.begin() + index);
    }

}

