#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#ifndef __ANDROID__
    #include "Extensions.h"
#endif
#include <vulkan/vulkan.hpp>
#include "Shader.h"

#include "SysConfig.h"

class ShaderProgram
{
   //vulkan stuff


    std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
    VkPipeline                                   vkPipeline;
    VkDescriptorSetLayout                        vkDescriptorSetLayout;
    VkDescriptorPool                             vkDescriptorPool;
    //opengl
    GLuint program;
    bool isVulkanShader;

public:
    static const int VULKAN_BUFFER_COUNT = 3;
    VkBuffer                     vkVertexBuffers[VULKAN_BUFFER_COUNT];
    VkDeviceMemory               vkVertexBuffersMemory[VULKAN_BUFFER_COUNT];
    VkDeviceSize                 vkBufferOffset[VULKAN_BUFFER_COUNT];

    VkPipelineLayout vkPipelineLayout;

    VkDescriptorSet  vkDS;

    void create(bool useVulkan);
    void destroy(VkDevice* vkDevice = nullptr);
    void getLog(char* string, int len);
    void attach(Shader& sh);
    //only for opengl
    void link();
    //for Vulkan only
    void buildVkPipeline(VkDevice* device,
                         VkPhysicalDevice* physical,
                         VkRenderPass* pass,
                         SystemConfig& config,
                         bool needUvs,
                         bool needAlphaBlend);

    void use(VkCommandBuffer* vkCmd = nullptr);

    int  getUniformID(const char* name);
    int  getAttributeID(const char* name);

    VkPipelineLayout* getVkPipelineLayout() {return &vkPipelineLayout;}

    ShaderProgram()
    {
        program = 0;
    }

};


#endif //_SHADER_PROGRAM_H
