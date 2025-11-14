#pragma once

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <SDL.h>
        #include <SDL_vulkan.h>
        #include <SDL_opengl.h>
        #include <vulkan/vulkan.hpp>
    #else
        #include <SDL2/SDL.h>
        #include <SDL2/SDL_opengl.h>
    #endif
#elif ANDROID
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <vulkan/vulkan.h>
    #include <vector>
#else
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_vulkan.h>
    #include <SDL2/SDL_opengl.h>
    #include <vulkan/vulkan.hpp>
#endif

#include "VulkanTexture.h"

class RenderTexture
{
public:

    void create(uint32_t width,
                uint32_t height,
                uint8_t filter,
                bool isVulkan = false,
                VkDevice* device = nullptr,
                VkPhysicalDevice* physical = nullptr,
                size_t vkSwapChainImageCount = 0);

    void bind(VkCommandBuffer* vkCmd = 0);
    void unbind(VkCommandBuffer* vkCmd = 0);

    GLuint getGLTexture(){return fboTexture;}

    void getVulkanTexture(VulkanTexture& tex);

    void destroy();

private:

    std::vector<VkFramebuffer>   vkSwapChainFramebuffers;
    VkRenderPass                 vkRenderPass;
    VkImage                      vkImage;
    VkImageView                  vkImageView;
    VkDeviceMemory               vkTextureMemory;

    uint32_t _width;
    uint32_t _height;

    GLuint fbo;
    GLuint fboTexture;

    bool useVulkan;

};
