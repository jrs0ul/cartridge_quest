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
#else
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_vulkan.h>
  #include <SDL2/SDL_opengl.h>
  #include <vulkan/vulkan.hpp>
#endif


class RenderTexture
{
public:

    void create(uint32_t width, uint32_t height, uint8_t filter, bool isVulkan = false);

    void bind();
    void unbind();

    GLuint getGLTexture(){return fboTexture;}

    void destroy();

private:

    uint32_t _width;
    uint32_t _height;

    GLuint fbo;
    GLuint fboTexture;

    bool useVulkan;

};
