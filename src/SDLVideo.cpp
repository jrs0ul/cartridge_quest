/*
 The Disarray 
 by jrs0ul(jrs0ul ^at^ gmail ^dot^ com) 2025
 -------------------------------------------
 SDL window
 */

#include "SDLVideo.h"
#include <cstdio>

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <SDL_opengl.h>
    #else
        #include <SDL2/SDL_opengl.h>
    #endif
#else
    #ifdef __APPLE__
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
            #include <OpenGLES/ES1/gl.h>
            #include <OpenGLES/ES1/glext.h>
        #else
              #include <SDL/SDL_opengl.h>
        #endif
    #elif __ANDROID__
            #include <GLES2/gl2.h>
            #include <GLES2/gl2ext.h>
    #else
            #include <SDL2/SDL_opengl.h>
    #endif
#endif

#include "MathTools.h"


void SDLVideo::createSemaphore(VkSemaphore *semaphore)
{
    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(vkDevice, &createInfo, nullptr, semaphore);
}


void SDLVideo::createBuffer(VkDevice& device,
                            VkPhysicalDevice& physical,
                            VkDeviceSize size, 
                            VkBufferUsageFlags usage, 
                            VkMemoryPropertyFlags properties, 
                            VkBuffer& buffer, 
                            VkDeviceMemory& bufferMemory) 
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physical, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t SDLVideo::findMemoryType(VkPhysicalDevice& physical, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}



void SDLVideo::createImage(VkDevice& device,
                           VkPhysicalDevice& physical,
                           uint32_t width,
                           uint32_t height,
                           VkFormat format,
                           VkImageTiling tiling,
                           VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties,
                           VkImage& image,
                           VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physical, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView SDLVideo::createImageView(VkDevice& vkDevice, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}


VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
{
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthFormat = format;
            return true;
        }
    }

    return false;
}

bool SDLVideo::initWindow(const char * title,
                          const char * iconPath,
                          bool isWindowed,
                          bool useVulkan)
{


    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        return false;
    }

    int rSize, gSize, bSize, aSize;
    rSize = gSize = bSize = aSize = 8;
    int buffsize = ((rSize + gSize + bSize + aSize + 7) & 0xf8);

    if (!useVulkan)
    {
        printf("Renderer: OPENGL\n");

        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 8);
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rSize );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, gSize );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, bSize );
        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, aSize );
        SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, buffsize );

        _flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    }
    else  //Vulkan
    {
        printf("Renderer: VULKAN\n");
        SDL_Vulkan_LoadLibrary(nullptr);

        _flags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;

    }

    if (!isWindowed){
        _flags = _flags |SDL_WINDOW_FULLSCREEN;
    }

    icon = SDL_LoadBMP(iconPath);

    mainWindow = SDL_CreateWindow(title, 
                         SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED,
                         _width,
                         _height,
                         _flags );
    if( mainWindow == 0 )
    {
        puts("can't set video mode");
        return false;
    }

    if (icon)
    {
         SDL_SetWindowIcon(mainWindow, icon);
    }

    if (!useVulkan)
    {
        SDL_GL_CreateContext(mainWindow);
    }
    else //  Vulkan
    {
        uint32_t extensionCount;
        const char** extensionNames = 0;

        SDL_Vulkan_GetInstanceExtensions(mainWindow, &extensionCount, nullptr);
        extensionNames = new const char *[extensionCount];
        SDL_Vulkan_GetInstanceExtensions(mainWindow, &extensionCount, extensionNames);

        const VkInstanceCreateInfo instanceInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
            nullptr,                                // pNext
            0,                                      // flags
            nullptr,                                // pApplicationInfo
            0,                                      // enabledLayerCount
            nullptr,                                // ppEnabledLayerNames
            extensionCount,                         // enabledExtensionCount
            extensionNames,                         // ppEnabledExtensionNames
        };

        VkResult res = vkCreateInstance(&instanceInfo, nullptr, &vkInstance);

        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vulkan instance!");
        }

        VkSurfaceKHR surface;
        SDL_Vulkan_CreateSurface(mainWindow, vkInstance, &surface);

        uint32_t physicalDeviceCount;
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.data());

        bool physicalDeviceAssigned = false;

        for (auto device : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                printf("We'll be using: %s!\n", properties.deviceName);
                vkPhysicalDevice = device;
                physicalDeviceAssigned = true;
                break;
            }
        }

        if (!physicalDeviceAssigned)
        {
            for (auto device : physicalDevices)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(device, &properties);


                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                {
                    printf("We'll be using: %s!\n", properties.deviceName);
                    vkPhysicalDevice = device;
                    physicalDeviceAssigned = true;
                    break;
                }
            }
        }


        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t graphicsQueueIndex = UINT32_MAX;
        uint32_t presentQueueIndex = UINT32_MAX;

        VkBool32 support;
        uint32_t i = 0;

        for (VkQueueFamilyProperties queueFamily : queueFamilies) 
        {
            if (graphicsQueueIndex == UINT32_MAX && 
                    queueFamily.queueCount > 0 && 
                    queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueueIndex = i;
            }
            if (presentQueueIndex == UINT32_MAX) 
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, surface, &support);

                if(support)
                {
                    presentQueueIndex = i;
                }
            }
            ++i;
        }

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // flags
            graphicsQueueIndex,                         // graphicsQueueIndex
            1,                                          // queueCount
            &queuePriority,                             // pQueuePriorities
        };

        VkPhysicalDeviceFeatures deviceFeatures = {};
        const char* deviceExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,   // sType
            nullptr,                                // pNext
            0,                                      // flags
            1,                                      // queueCreateInfoCount
            &queueInfo,                             // pQueueCreateInfos
            0,                                      // enabledLayerCount
            nullptr,                                // ppEnabledLayerNames
            1,                                      // enabledExtensionCount
            deviceExtensionNames,                   // ppEnabledExtensionNames
            &deviceFeatures,                        // pEnabledFeatures
        };

        vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice);

        vkGetDeviceQueue(vkDevice, graphicsQueueIndex, 0, &vkGraphicsQueue);

        vkGetDeviceQueue(vkDevice, presentQueueIndex, 0, &vkPresentQueue);


        //Let's create a swap chain
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface, &vkSurfaceCapabilities);

        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            surfaceFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, surfaceFormats.data());
        }

        std::vector<VkPresentModeKHR> presentModes;
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, presentModes.data());
        }

        vkSurfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);


        int width, height = 0;
        SDL_Vulkan_GetDrawableSize(mainWindow, &width, &height);
        width = CLAMP((uint32_t)width, vkSurfaceCapabilities.minImageExtent.width, vkSurfaceCapabilities.maxImageExtent.width);
        height = CLAMP((uint32_t)height, vkSurfaceCapabilities.minImageExtent.height, vkSurfaceCapabilities.maxImageExtent.height);
        vkSwapchainSize.width = width;
        vkSwapchainSize.height = height;

        uint32_t imageCount = vkSurfaceCapabilities.minImageCount + 1;

        if (vkSurfaceCapabilities.maxImageCount > 0 && imageCount > vkSurfaceCapabilities.maxImageCount)
        {
            imageCount = vkSurfaceCapabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount    = vkSurfaceCapabilities.minImageCount;
        createInfo.imageFormat      = vkSurfaceFormat.format;
        createInfo.imageColorSpace  = vkSurfaceFormat.colorSpace;
        createInfo.imageExtent      = vkSwapchainSize;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {graphicsQueueIndex, presentQueueIndex};

        if (graphicsQueueIndex != presentQueueIndex)
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform   = vkSurfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;


        vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &vkSwapchain);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapchainImageCount, nullptr);
        vkSwapchainImages.resize(vkSwapchainImageCount);
        vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapchainImageCount, vkSwapchainImages.data());


        vkSwapchainImageViews.resize(vkSwapchainImages.size());

        for (uint32_t i = 0; i < vkSwapchainImages.size(); i++)
        {
            vkSwapchainImageViews[i] = createImageView(vkDevice, vkSwapchainImages[i], vkSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        getSupportedDepthFormat(vkPhysicalDevice, &vkDepthFormat);

        createImage(vkDevice,
                    vkPhysicalDevice,
                    vkSwapchainSize.width,
                    vkSwapchainSize.height,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    depthImage,
                    depthImageMemory);

        VkImageView depthImageView = createImageView(vkDevice, depthImage, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT);


        //render pass

        std::vector<VkAttachmentDescription> attachments(2);

        attachments[0].format         = vkSurfaceFormat.format;
        attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments[1].format         = vkDepthFormat;
        attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 1;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        std::vector<VkSubpassDependency> dependencies(1);

        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = 0;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(vkDevice, &renderPassInfo, nullptr, &vkRenderPass);

        //framebuffers
        vkSwapchainFramebuffers.resize(vkSwapchainImageViews.size());

        for (size_t i = 0; i < vkSwapchainImageViews.size(); i++)
        {
            std::vector<VkImageView> attachments(2);
            attachments[0] = vkSwapchainImageViews[i];
            attachments[1] = depthImageView;

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vkRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = vkSwapchainSize.width;
            framebufferInfo.height = vkSwapchainSize.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &vkSwapchainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }

        //command pool

        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        poolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
        vkCreateCommandPool(vkDevice, &poolCreateInfo, nullptr, &vkCommandPool);

        //command buffers
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = vkCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = vkSwapchainImageCount;

        vkCommandBuffers.resize(vkSwapchainImageCount);
        vkAllocateCommandBuffers(vkDevice, &allocateInfo, vkCommandBuffers.data());

        createSemaphore(&vkImageAvailableSemaphore);
        createSemaphore(&vkRenderingFinishedSemaphore);

        vkFences.resize(vkSwapchainImageCount);

        for(uint32_t i = 0; i < vkSwapchainImageCount; i++)
        {
            VkFenceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(vkDevice, &createInfo, nullptr, &vkFences[i]);
        }

    }

    printf("Context created\n");

    return true;


}
//-------------------------------------------
void SDLVideo::setMetrics(unsigned w, unsigned h){
    _width = w;
    _height = h;
}

//--------------------------------------------
void SDLVideo::quit(bool useVulkan)
{
    if (useVulkan)
    {

        for(uint32_t i = 0; i < vkSwapchainImageCount; ++i)
        {
            vkDestroyFence(vkDevice, vkFences[i], nullptr);
        }

        vkDestroySemaphore(vkDevice, vkRenderingFinishedSemaphore, nullptr);
        vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore, nullptr);

        vkFreeCommandBuffers(vkDevice, vkCommandPool, vkCommandBuffers.size(), vkCommandBuffers.data());
        vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);

        for (size_t i = 0; i < vkSwapchainImageViews.size(); ++i)
        {
            vkDestroyFramebuffer(vkDevice, vkSwapchainFramebuffers[i], nullptr);
        }

        vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
        vkFreeMemory(vkDevice, depthImageMemory, nullptr);
        vkDestroyImage(vkDevice, depthImage, nullptr);

        for (uint32_t i = 0; i < vkSwapchainImages.size(); ++i)
        {
            vkDestroyImageView(vkDevice, vkSwapchainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
        vkDestroyDevice(vkDevice, nullptr);
        vkDestroyInstance(vkInstance, nullptr);
    }

    SDL_DestroyWindow(mainWindow);

    if (useVulkan)
    {
        SDL_Vulkan_UnloadLibrary();
    }

    SDL_Quit();
}

void SDLVideo::beginRenderPass(bool useVulkan)
{
    if (!useVulkan)
    {
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
        VkAcquireNextImage();
        VkResetCommandBuffer();
        VkBeginCommandBuffer();
        VkBeginRenderPass({0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0});
    }
}

void SDLVideo::swap(bool useVulkan)
{
    if (!useVulkan)
    {
        glFlush();
        SDL_GL_SwapWindow(mainWindow);
    }
    else
    {
        VkEndRenderPass();
        VkEndCommandBuffer();
        VkQueueSubmit();
        VkQueuePresent();
    }
}


void SDLVideo::VkAcquireNextImage()
{
    vkAcquireNextImageKHR(vkDevice,
                          vkSwapchain,
                          UINT64_MAX,
                          vkImageAvailableSemaphore,
                          VK_NULL_HANDLE,
                          &vkFrameIndex);

    vkWaitForFences(vkDevice, 1, &vkFences[vkFrameIndex], VK_FALSE, UINT64_MAX);
    vkResetFences(vkDevice, 1, &vkFences[vkFrameIndex]);

    vkCommandBuffer = vkCommandBuffers[vkFrameIndex];
    vkImage = vkSwapchainImages[vkFrameIndex];
}

void SDLVideo::VkResetCommandBuffer()
{
    vkResetCommandBuffer(vkCommandBuffer, 0);
}

void SDLVideo::VkBeginCommandBuffer()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
}

void SDLVideo::VkBeginRenderPass(VkClearColorValue clearColor, VkClearDepthStencilValue clearDepthStencil)
{
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = vkRenderPass;
    render_pass_info.framebuffer       = vkSwapchainFramebuffers[vkFrameIndex];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = vkSwapchainSize;

    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDepthStencil;

    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(vkCommandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void SDLVideo::VkEndRenderPass()
{
    vkCmdEndRenderPass(vkCommandBuffer);
}

void SDLVideo::VkEndCommandBuffer()
{
    vkEndCommandBuffer(vkCommandBuffer);
}

VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

void SDLVideo::VkQueueSubmit()
{
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vkImageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vkRenderingFinishedSemaphore;
    vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkFences[vkFrameIndex]);
}

void SDLVideo::VkQueuePresent()
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vkRenderingFinishedSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkSwapchain;
    presentInfo.pImageIndices = &vkFrameIndex;
    vkQueuePresentKHR(vkPresentQueue, &presentInfo);

    //vkQueueWaitIdle(vkPresentQueue);
}

