#pragma once


//Vulkan texture
struct VulkanTexture
{
    VkImage         vkImage;
    VkImageView     vkImageView;
    VkSampler       vkSampler;
    VkDeviceMemory  vkTextureMemory;
};

