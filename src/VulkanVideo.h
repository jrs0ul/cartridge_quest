#pragma once

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <SDL.h>
        #include <SDL_vulkan.h>
        #include <vulkan/vulkan.hpp>
    #else
        #include <SDL2/SDL.h>
    #endif
#elif ANDROID
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <vulkan/vulkan.h>
    #include <vector>
#else
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_vulkan.h>
  #include <vulkan/vulkan.hpp>
#endif


class VulkanVideo
{

public:
    static uint32_t findMemoryType(VkPhysicalDevice& physical, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    static void createImage(VkDevice& device,
                            VkPhysicalDevice& physical,
                            uint32_t width, uint32_t height,
                            VkFormat format, VkImageTiling tiling, 
                            VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, 
                            VkDeviceMemory& imageMemory);

    static void createBuffer(VkDevice& device,
                            VkPhysicalDevice& physical,
                            VkDeviceSize size,
                            VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties,
                            VkBuffer& buffer,
                            VkDeviceMemory& bufferMemory);

    static VkImageView createImageView(VkDevice& device,
                                       VkImage& image,
                                       VkFormat format,
                                       VkImageAspectFlags aspectFlags);

};
