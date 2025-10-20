/*    Copyright (C) 2025 by jrs0ul                                          *
 *   jrs0ul@gmail.com                                                      *
*/

#include "ShaderProgram.h"
#include <cstdio>

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))

#endif

void ShaderProgram::create(bool useVulkan)
{

    isVulkanShader = useVulkan;

    if (!useVulkan)
    {
        program = glCreateProgram();
    }
}

void ShaderProgram::destroy()
{
    glDeleteProgram(program);
}

void ShaderProgram::getLog(char* string, int len)
{
#ifndef __ANDROID__
    glGetInfoLogARB(program, len, 0, string);
#else
    glGetShaderInfoLog(program, len, 0, string);
#endif
}

void ShaderProgram::attach(Shader & sh)
{
    if (!isVulkanShader)
    {
        glAttachShader(program, sh.id);
    }
    else //VULKAN
    {
        VkPipelineShaderStageCreateInfo vkShaderStage{};
        vkShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vkShaderStage.pNext = nullptr;
        vkShaderStage.stage = (sh.type == VERTEX_SHADER) ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
        vkShaderStage.module = sh.vkShaderModule;
        vkShaderStage.pName = "main";


        vkShaderStages.push_back(vkShaderStage);
    }
}

void ShaderProgram::link()
{
    glLinkProgram(program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
#ifdef __ANDROID__
        LOGI("Linking failed!");
#else
        printf("Linking failed!!!1\n");
#endif

    }
    else
    {
        //printf("All good\n");
    }
}

uint32_t findMemoryType(VkPhysicalDevice* vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}




void ShaderProgram::buildVkPipeline(VkDevice* device, VkPhysicalDevice* physical, VkRenderPass* pass, bool needUvs)
{
    const int MAX_VERTEX_BUFFER_SIZE = sizeof(float) * 4 * 10000;

    for (int i = 0; i < 3; ++i)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = MAX_VERTEX_BUFFER_SIZE;
        bufferInfo.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*device, &bufferInfo, nullptr, &vkVertexBuffers[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*device, vkVertexBuffers[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physical, memRequirements.memoryTypeBits, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(*device, &allocInfo, nullptr, &vkVertexBuffersMemory[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(*device, vkVertexBuffers[i], vkVertexBuffersMemory[i], 0);
    }


    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &vkPipelineLayout);


    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;


    const int bindingCount = (needUvs) ? 3 : 2;

    for (int i = 0; i < bindingCount; ++i)
    {
        VkVertexInputBindingDescription b = {};
        b.binding = i;
        b.stride = 8;

        if ((needUvs && i == 2) || (!needUvs && i == 1)) //for colors
        {
            b.stride = 16;
        }

        b.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindings.push_back(b);
    }


    //pos
    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    positionAttribute.offset = 0;

    //uvs
    VkVertexInputAttributeDescription uvsAttribute = {};
    uvsAttribute.binding = 1;
    uvsAttribute.location = 1;
    uvsAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvsAttribute.offset = 0;

    //Color
    VkVertexInputAttributeDescription colorAttribute = {};
    colorAttribute.binding = (needUvs) ? 2 : 1;
    colorAttribute.location = (needUvs) ? 2 : 1;
    colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    colorAttribute.offset = 0;

    attributes.push_back(positionAttribute);

    if (needUvs)
    {
        attributes.push_back(uvsAttribute);
    }

    attributes.push_back(colorAttribute);


    VkPipelineVertexInputStateCreateInfo   vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.vertexBindingDescriptionCount = bindings.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;


    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = {640, 360};

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 640;
    viewport.height = 360;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkPipelineViewportStateCreateInfo      viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = nullptr;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo   multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = nullptr;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo    colorBlending = {};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext           = nullptr;
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.logicOp         = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext               = nullptr;
    pipelineInfo.stageCount          = vkShaderStages.size();
    pipelineInfo.pStages             = vkShaderStages.data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.layout              = vkPipelineLayout;
    pipelineInfo.renderPass          = *pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(*device,
                                  VK_NULL_HANDLE,
                                  1,
                                  &pipelineInfo,
                                  nullptr,
                                  &vkPipeline) != VK_SUCCESS)
    {
        printf("Failed to create a pipeline\n");
    }
}

void ShaderProgram::use(VkCommandBuffer* vkCmd)
{
    if (!isVulkanShader)
    {
        glUseProgram(program);
    }
    else // VULKAN
    {
        vkCmdBindPipeline(*vkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
    }
}

int ShaderProgram::getUniformID(const char* name)
{
    return  glGetUniformLocation(program, name);
}

int ShaderProgram::getAttributeID(const char* name)
{
    return glGetAttribLocation(program, name);
}
