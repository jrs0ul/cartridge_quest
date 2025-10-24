/*
 The Disarray 
 by jrs0ul(jrs0ul ^at^ gmail ^dot^ com) 2025
 -------------------------------------------
 Sprite batcher
 mod. 2025.09.28
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




GLuint PicsContainer::getGLName(unsigned long index)
{
    if (index < glTextures.count())
    {
        return glTextures[index];
    }

    return 0;
}
//-----------------------------

PicData* PicsContainer::getInfo(unsigned long index)
{
    if (index < vkTextures.size())
    {
        return &PicInfo[index];
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
bool PicsContainer::load(const char* list, 
                         bool useVulkan,
                         VkDevice* vkDevice,
                         VkPhysicalDevice* physical,
                         VkCommandPool* vkCommandPool,
                         VkQueue* vkGraphicsQueue
                         )
#else
bool PicsContainer::load(const char* list, AAssetManager* assman, 
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

    for (unsigned long i = 0; i < PicInfo.count(); i++)
    {

        Image newImg;
        unsigned short imageBits = 0;

#ifdef __ANDROID__
        if (!newImg.loadTga(PicInfo[i].name, imageBits, assman))
        {
            LOGI("%s not found or corrupted by M$\n", PicInfo[i].name);
        }
#else
        if (!newImg.loadTga(PicInfo[i].name, imageBits))
        {
            printf("%s not found or corrupted by M$\n", PicInfo[i].name);
        }
#endif

        PicInfo[i].width  = newImg.width;
        PicInfo[i].height = newImg.height;


        PicInfo[i].htilew  = PicInfo[i].twidth / 2.0f;
        PicInfo[i].htileh  = PicInfo[i].theight / 2.0f;
        PicInfo[i].vframes = PicInfo[i].height / PicInfo[i].theight;
        PicInfo[i].hframes = PicInfo[i].width / PicInfo[i].twidth;


        if (!useVulkan)
        {
            int filter = GL_NEAREST;

            if (PicInfo[i].filter)
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
            SDLVideo::createBuffer(*vkDevice, *physical, imageSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(*vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, newImg.data, static_cast<size_t>(imageSize));
            vkUnmapMemory(*vkDevice, stagingBufferMemory);


            Texture t;

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

            if (PicInfo[i].filter)
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
void PicsContainer::bindTexture(unsigned long index,
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
void PicsContainer::draw(
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
    if (!flipColors){
        nb.upColor[0] = upColor;
        nb.upColor[1] = upColor;
        nb.dwColor[0] = dwColor;
        nb.dwColor[1] = dwColor;
    }
    else{
        nb.upColor[0] = upColor;
        nb.dwColor[0] = upColor;
        nb.upColor[1] = dwColor;
        nb.dwColor[1] = dwColor;
    }
        batch.add(nb);
}
//---------------------------------------------------
void PicsContainer::drawVA(void * vertices, 
                           void * uvs,
                           void *colors,
                           unsigned uvsCount,
                           unsigned vertexCount,
                           ShaderProgram* shader,
                           bool useVulkan,
                           VkCommandBuffer* vkCmd,
                           VkDevice* vkDevice)
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
        void* vertData;
        vkMapMemory(*vkDevice,
                    shader->vkVertexBuffersMemory[0],
                    vkVertexBufferOffset,
                    vertexCount * sizeof(float),
                    0,
                    &vertData);
        memcpy(vertData, vertices, vertexCount * sizeof(float));
        vkUnmapMemory(*vkDevice, shader->vkVertexBuffersMemory[0]);

        if (uvsCount)
        {
            void* uvsData;
            vkMapMemory(*vkDevice,
                        shader->vkVertexBuffersMemory[1],
                        vkUVsBufferOffset,
                        sizeof(float) * vertexCount,
                        0,
                        &uvsData);
            memcpy(uvsData, uvs, sizeof(float) * vertexCount);
            vkUnmapMemory(*vkDevice, shader->vkVertexBuffersMemory[1]);
        }

        void* colorData;


        vkMapMemory(*vkDevice,
                    (uvsCount) ? shader->vkVertexBuffersMemory[2] : shader->vkVertexBuffersMemory[1],
                    vkColorBufferOffset,
                    sizeof(float) * vertexCount * 2,
                    0,
                    &colorData);
        memcpy(colorData, colors, sizeof(float) * vertexCount * 2);
        vkUnmapMemory(*vkDevice, (uvsCount) ? shader->vkVertexBuffersMemory[2] : shader->vkVertexBuffersMemory[1]);

        vkCmdBindVertexBuffers(*vkCmd, 0, 1, &shader->vkVertexBuffers[0], &vkVertexBufferOffset);

        if (uvsCount)
        {
            vkCmdBindVertexBuffers(*vkCmd, 1, 1, &shader->vkVertexBuffers[1], &vkUVsBufferOffset);
            vkCmdBindVertexBuffers(*vkCmd, 2, 1, &shader->vkVertexBuffers[2], &vkColorBufferOffset);
        }
        else
        {
            vkCmdBindVertexBuffers(*vkCmd, 1, 1, &shader->vkVertexBuffers[1], &vkColorBufferOffset);
        }

        vkCmdBindDescriptorSets(*vkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->vkPipelineLayout, 0, 1, &shader->vkDS, 0, nullptr);

        vkCmdDraw(*vkCmd, vertexCount / 2, 1, 0, 0);

        if (uvsCount)
        {
            vkUVsBufferOffset += vertexCount * sizeof(float);
        }

        vkColorBufferOffset += (vertexCount * sizeof(float)*2);
        vkVertexBufferOffset += vertexCount * sizeof(float);

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
                               (startx*1.0f)/(p->width*1.0f),
                               ((startx+p->twidth)*1.0f)/(p->width*1.0f),
                               (( p->height - starty) * 1.0f ) / ( p->height * 1.0f ),//- 0.0001f,
                               (( p->height - starty - p->theight ) * 1.0f) / (p->height * 1.0f)
                              );
    return result;
}

//----------------------------------------------------------
void PicsContainer::drawBatch(ShaderProgram * justColor,
                              ShaderProgram * uvColor,
                              int method,
                              bool useVulkan,
                              VkCommandBuffer* vkCmd,
                              VkDevice*        vkDevice)
{

        //printf("=============\n");
        vkVertexBufferOffset = 0;
        vkUVsBufferOffset = 0;
        vkColorBufferOffset = 0;

        switch(method){
              //TODO: complete VA
        default:{
                DArray<float> vertices;
                DArray<float> uvs;
                float * colors = 0;
                unsigned colorIndex = 0;

                if (batch.count())
                {
                    colors = (float*)malloc(sizeof(float) * 24 * batch.count());
                }

                Vector3D uv(0.0f, 0.0f, 0.0f, 0.0f);
                float htilew, htileh;
                float twidth, theight;


                long texIndex = -1;

                for (unsigned long i = 0; i < batch.count(); i++){

                    PicData * p = 0;

                    htilew = 0.5f; htileh = 0.5f;
                    twidth = 1; theight = 1;

                    if ((batch[i].textureIndex >= 0) && ( batch[i].textureIndex < (long)count()))
                    {
                        p = &PicInfo[batch[i].textureIndex];
                    }

                    if (p)
                    {
                        if (p->sprites.count()) //iregular sprites in atlas
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
                        && (vertices.count() > 0))
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
                                uvs.destroy();
                            }
                        }

                        drawVA(vertices.getData(), uvs.getData(), colors,
                               uvs.count(), vertices.count(), currentShader, useVulkan, vkCmd, vkDevice);

                        vertices.destroy();
                        uvs.destroy();
                        colorIndex = 0;

                }
                texIndex = batch[i].textureIndex;

                //append to arrays
                uvs.add(uv.v[0]); uvs.add(uv.v[2]);
                uvs.add(uv.v[1]); uvs.add(uv.v[2]);
                uvs.add(uv.v[1]); uvs.add(uv.v[3]);
                uvs.add(uv.v[0]); uvs.add(uv.v[2]);
                uvs.add(uv.v[1]); uvs.add(uv.v[3]);
                uvs.add(uv.v[0]); uvs.add(uv.v[3]); 

                //----
                    memcpy(&colors[colorIndex], batch[i].upColor[0].c,
                           sizeof(float) * 4);
                    colorIndex += 4;

                    memcpy(&colors[colorIndex], batch[i].upColor[1].c, 
                           sizeof(float) * 4);
                    colorIndex += 4;

                    memcpy(&colors[colorIndex], batch[i].dwColor[1].c, 
                           sizeof(float) * 4);
                    colorIndex += 4;

                    memcpy(&colors[colorIndex], batch[i].upColor[0].c, 
                           sizeof(float) * 4);
                    colorIndex += 4;

                    memcpy(&colors[colorIndex], batch[i].dwColor[1].c, 
                           sizeof(float) * 4);
                    colorIndex += 4;

                    memcpy(&colors[colorIndex], batch[i].dwColor[0].c, 
                           sizeof(float) * 4);
                    colorIndex += 4;

                //---
                if (batch[i].rotationAngle == 0.0f){
                    if (batch[i].useCenter){
                        float hwidth = htilew * batch[i].scaleX;
                        float hheight = htileh * batch[i].scaleY;


                        vertices.add(batch[i].x - hwidth); 
                        vertices.add(batch[i].y - hheight);

                        vertices.add(batch[i].x + hwidth); 
                        vertices.add(batch[i].y - hheight);

                        vertices.add(batch[i].x + hwidth); 
                        vertices.add(batch[i].y + hheight);

                        vertices.add(batch[i].x - hwidth); 
                        vertices.add(batch[i].y - hheight);

                        vertices.add(batch[i].x + hwidth); 
                        vertices.add(batch[i].y + hheight);

                        vertices.add(batch[i].x - hwidth); 
                        vertices.add(batch[i].y + hheight);
                    }
                    else{

                        vertices.add(batch[i].x);
                        vertices.add(batch[i].y);

                        vertices.add(batch[i].x + twidth * batch[i].scaleX);
                        vertices.add(batch[i].y);

                        vertices.add(batch[i].x + twidth * batch[i].scaleX); 
                        vertices.add(batch[i].y + theight * batch[i].scaleY);

                        vertices.add(batch[i].x);
                        vertices.add(batch[i].y);

                        vertices.add(batch[i].x + twidth * batch[i].scaleX); 
                        vertices.add(batch[i].y + theight * batch[i].scaleY);


                        vertices.add(batch[i].x); 
                        vertices.add(batch[i].y + theight * batch[i].scaleY);
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


                        vertices.add(batch[i].x + cos_rot_w - sin_rot_h); 
                        vertices.add(batch[i].y + sin_rot_w + cos_rot_h);

                        vertices.add(batch[i].x - cos_rot_w - sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w + cos_rot_h);

                        vertices.add(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.add(batch[i].x + cos_rot_w - sin_rot_h); 
                        vertices.add(batch[i].y + sin_rot_w + cos_rot_h);

                        vertices.add(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.add(batch[i].x + cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y + sin_rot_w - cos_rot_h);
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

                        vertices.add(batch[i].x); 
                        vertices.add(batch[i].y);

                        vertices.add(batch[i].x - cos_rot_w - sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w + cos_rot_h);

                        vertices.add(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.add(batch[i].x); 
                        vertices.add(batch[i].y);

                        vertices.add(batch[i].x - cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y - sin_rot_w - cos_rot_h);

                        vertices.add(batch[i].x + cos_rot_w + sin_rot_h); 
                        vertices.add(batch[i].y + sin_rot_w - cos_rot_h);

                    }

                }
            } //for

            if (vertices.count() > 0){

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
                        uvs.destroy();
                    }
                }

                drawVA(vertices.getData(), uvs.getData(), colors,
                       uvs.count(), vertices.count(), currentShader, useVulkan, vkCmd, vkDevice);


                vertices.destroy();
                uvs.destroy();
                colorIndex = 0;
            }

            if (colors)
            {
                free(colors);
            }
        }
    }

    batch.destroy();
}
//-----------------------------------------------------
void PicsContainer::resizeContainer(unsigned long index,
                                    int twidth, int theight, int filter,
                                    const char * name,
                                    bool createTextures,
                                    GLuint texname)
{

    if (PicInfo.count() < index + 1)
    {

        GLuint glui = texname;
        PicData p;
        p.twidth = twidth;
        p.theight = theight;
        p.filter = filter;

        for (unsigned i = PicInfo.count(); i < index + 1; i++)
        {
            PicInfo.add(p);
            glTextures.add(glui);
        }

        if (createTextures)
        {
            glGenTextures(1, ((GLuint *)glTextures.getData()) + index);
        }

        char * copy = (char*)malloc(strlen(name)+1);
        strcpy(copy, name);
        char * res = 0;
        res = strtok(copy, "/");

        while (res)
        {
            strcpy(PicInfo[index].name, res);
            res = strtok(0, "/");
        }

        free(copy);

    }
    else
    {
        PicData * pp = &PicInfo[index];
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
            glDeleteTextures(1, ((GLuint *)glTextures.getData()) + index);
        }

        if (createTextures)
        {
            glGenTextures(1, ((GLuint *)glTextures.getData()) + index);
        }
        else
        {
            *(((GLuint *)glTextures.getData()) + index) = texname;
        }

    }


}

//-----------------------------------------------------
#ifndef __ANDROID__
bool PicsContainer::loadFile(const char* file,
                             unsigned long index,
                             int twidth,
                             int theight,
                             int filter)
#else
bool PicsContainer::loadFile(const char* file,
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

        PicInfo[index].width = naujas.width;
        PicInfo[index].height = naujas.height;


        PicInfo[index].htilew =PicInfo[index].twidth/2.0f;
        PicInfo[index].htileh =PicInfo[index].theight/2.0f;
        PicInfo[index].vframes=PicInfo[index].height/PicInfo[index].theight;
        PicInfo[index].hframes=PicInfo[index].width/PicInfo[index].twidth;

        int filtras = GL_NEAREST;

        if (PicInfo[index].filter)
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
void PicsContainer::makeTexture(Image& img,
                                const char * name,
                                unsigned long index,
                                int twidth,
                                int theight,
                                int filter)
{

    resizeContainer(index, twidth, theight, filter, name);

    PicInfo[index].width = img.width;
    PicInfo[index].height = img.height;


    PicInfo[index].htilew = PicInfo[index].twidth/2.0f;
    PicInfo[index].htileh = PicInfo[index].theight/2.0f;
    PicInfo[index].vframes = PicInfo[index].height/PicInfo[index].theight;
    PicInfo[index].hframes = PicInfo[index].width/PicInfo[index].twidth;

    int filtras = GL_NEAREST;

    if (PicInfo[index].filter)
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
bool PicsContainer::loadFile(unsigned long index,
                             const char * BasePath)
{

    Image naujas;

    unsigned short imageBits = 0;


    char dir[512];
    char buf[1024];

    sprintf(dir, "%spics/", BasePath);
    sprintf(buf, "%s%s", dir, PicInfo[index].name);

#ifndef __ANDROID__
    if (!naujas.loadTga(buf, imageBits)){
#else
    if (!naujas.loadTga(buf, imageBits, 0)){
#endif

        sprintf(buf, "base/pics/%s", PicInfo[index].name);
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

    PicInfo[index].width = naujas.width;
    PicInfo[index].height = naujas.height;


    PicInfo[index].htilew = PicInfo[index].twidth / 2.0f;
    PicInfo[index].htileh = PicInfo[index].theight / 2.0f;
    PicInfo[index].vframes = PicInfo[index].height / PicInfo[index].theight;
    PicInfo[index].hframes = PicInfo[index].width / PicInfo[index].twidth;

    int filtras = GL_NEAREST;
    if (PicInfo[index].filter)
        filtras = GL_LINEAR;


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
void PicsContainer::attachTexture(GLuint textureID, unsigned long index,
                                  int width, int height,
                                 int twidth, int theight, int filter)
{

    resizeContainer(index, twidth, theight, filter, "lol", false, textureID);
    PicInfo[index].width = width;
    PicInfo[index].height = height;


    PicInfo[index].htilew = PicInfo[index].twidth / 2.0f;
    PicInfo[index].htileh = PicInfo[index].theight / 2.0f;
    PicInfo[index].vframes = PicInfo[index].height / PicInfo[index].theight;
    PicInfo[index].hframes = PicInfo[index].width / PicInfo[index].twidth;


    int filtras = GL_NEAREST;

    if (PicInfo[index].filter)
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


//--------------------------------
int PicsContainer::findByName(const char* picname, bool debug){
    unsigned long start = 0;

    if (!PicInfo.count())
    {
        return -1;
    }

    while ((strcmp(PicInfo[start].name, picname) != 0) && (start < PicInfo.count()))
    {
        if (debug)
            puts(PicInfo[start].name);
        start++;
    }

    if (start == PicInfo.count())
    {
        return -1;
    }

    return start;
}
//---------------------------------------
#ifndef __ANDROID__
bool PicsContainer::initContainer(const char *list, bool useVulkan)
#else
bool PicsContainer::initContainer(const char *list, AAssetManager* assman, bool useVulkan)
#endif
{


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

                            data.sprites.add(spr);

                        }
                    }

                }

            }

#ifdef __ANDROID__
//            LOGI(">>>Image: name:%s width:%d height:%d filter%d\n", data.name, data.twidth, data.theight, data.filter);
#else

//            printf(">>>Image: name:%s width:%d height:%d filter%d\n", data.name, data.twidth, data.theight, data.filter);
#endif
            PicInfo.add(data);
        }

        if (!useVulkan)
        {
            for (unsigned long i = 0; i < PicInfo.count(); i++) 
            {
                GLuint glui = 0;
                glTextures.add(glui);
            }

            printf("Creating %lu opengl textures\n", PicInfo.count());
            glGenTextures(PicInfo.count(), (GLuint *)glTextures.getData());
        }
    }

    pictureList.destroy();

    return result;

}

//----------------------------------
void PicsContainer::destroy()
{
    for (unsigned long i = 0; i < glTextures.count(); i++)
    {
        if (glIsTexture(glTextures[i]))
        {
            glDeleteTextures(1, ((GLuint *)glTextures.getData()) + i);
        }
    }

    glTextures.destroy();
    batch.destroy();

    for (unsigned long i = 0; i < PicInfo.count(); ++i)
    {
        PicInfo[i].sprites.destroy();
    }

    PicInfo.destroy();
}

//-------------------------------
void PicsContainer::remove(unsigned long index)
{
    if (index < glTextures.count())
    {
        if (glIsTexture(glTextures[index]))
        {
            glDeleteTextures(1, ((GLuint *)glTextures.getData()) + index);
        }

        glTextures.remove(index);
        PicInfo.remove(index);
    }

}

