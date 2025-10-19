#ifndef _SHADER_PROGRAM_H
#define _SHADER_PROGRAM_H

#ifndef __ANDROID__
    #include "Extensions.h"
#endif
#include <vulkan/vulkan.hpp>
#include "Shader.h"

class ShaderProgram
{
    GLuint program;
    bool isVulkanShader;
//vulkan stuff
    std::vector<VkPipelineShaderStageCreateInfo> vkShaderStages;
    VkPipeline       vkPipeline;
    VkPipelineLayout vkPipelineLayout;

public:
    VkBuffer         vkVertexBuffers[3];
    VkDeviceMemory   vkVertexBuffersMemory[3];

    void create(bool useVulkan);
    void destroy();
    void getLog(char* string, int len);
    void attach(Shader& sh);
    //only for opengl
    void link();
    //for Vulkan only
    void buildVkPipeline(VkDevice* device, VkPhysicalDevice* physical, VkRenderPass* pass, bool needUvs);

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
