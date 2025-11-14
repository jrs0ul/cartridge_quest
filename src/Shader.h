#pragma once

#ifdef __ANDROID__
    #include <GLES2/gl2.h>
    #include <vulkan/vulkan.h>
    #include <android/asset_manager.h>
#else
    #include "Extensions.h"
    #include <vulkan/vulkan.hpp>
#endif

class ShaderProgram;


enum ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER
};

class Shader{
        GLuint id;
        ShaderType type;
        VkShaderModule vkShaderModule;

public:

        Shader(){
            id = 0;
        }
        //loads and compiles shader
#ifndef __ANDROID__
        bool loadGL(ShaderType shaderType, const char* path);
        bool loadVK(ShaderType shaderType, const char* path, VkDevice* device);
#else
        bool loadGL(ShaderType shaderType, const char* path, AAssetManager* man);
        bool loadVK(ShaderType shaderType, const char* path, VkDevice* device, AAssetManager* man);
#endif

        friend class ShaderProgram;

};

