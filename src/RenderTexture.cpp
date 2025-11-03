#include "RenderTexture.h"
#include "Extensions.h"


void RenderTexture::create(uint32_t width, uint32_t height, uint8_t filter, bool isVulkan)
{
    useVulkan = isVulkan;
    _width = width;
    _height = height;

    if (!isVulkan)
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == 1 ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == 1 ? GL_LINEAR : GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else  // VULKAN
    {
    }
}

void RenderTexture::bind()
{
    if (!useVulkan)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, _width, _height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

}

void RenderTexture::unbind()
{
    if (!useVulkan)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}


void RenderTexture::destroy()
{
    if (!useVulkan)
    {
        glDeleteFramebuffers(1, &fbo);
    }
}
