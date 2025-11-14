#include "Extensions.h"

const GLubyte * GLExtensionString = 0;

#ifndef __APPLE__
PFNGLGENBUFFERSPROC                  glGenBuffers = 0;
PFNGLDELETEBUFFERSPROC               glDeleteBuffers = 0;
PFNGLBINDBUFFERPROC                  glBindBuffer = 0;
PFNGLBUFFERDATAPROC                  glBufferData = 0;
PFNGLMAPBUFFERPROC                   glMapBuffer = 0;
PFNGLUNMAPBUFFERPROC                 glUnmapBuffer = 0;

PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer = 0;
PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray = 0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray = 0;


PFNGLCREATESHADERPROC                glCreateShader = 0; 
PFNGLSHADERSOURCEPROC                glShaderSource = 0;
PFNGLCOMPILESHADERPROC               glCompileShader = 0;
PFNGLGETSHADERIVPROC                 glGetShaderiv = 0;
PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog = 0;
PFNGLDELETESHADERPROC                glDeleteShader = 0;

PFNGLCREATEPROGRAMPROC               glCreateProgram = 0;
PFNGLDELETEPROGRAMPROC               glDeleteProgram = 0;
PFNGLATTACHSHADERPROC                glAttachShader = 0;
PFNGLLINKPROGRAMPROC                 glLinkProgram = 0;
PFNGLGETPROGRAMIVPROC                glGetProgramiv = 0;
PFNGLUSEPROGRAMPROC                  glUseProgram  = 0;
PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation = 0;
PFNGLGETATTRIBLOCATIONPROC           glGetAttribLocation = 0;
PFNGLUNIFORM1FPROC                   glUniform1f = 0;
PFNGLUNIFORM2FPROC                   glUniform2f = 0;
PFNGLUNIFORM3FPROC                   glUniform3f = 0;
PFNGLUNIFORM4FPROC                   glUniform4f = 0;
PFNGLUNIFORM1IPROC                   glUniform1i = 0;
PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv = 0;
#endif
PFNGLGETINFOLOGARBPROC               glGetInfoLogARB = 0;

#ifdef WIN32
PFNGLGENFRAMEBUFFERSEXTPROC          glGenFramebuffers = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC       glDeleteFramebuffers = 0;
PFNGLBINDFRAMEBUFFEREXTPROC          glBindFramebuffer = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC     glFramebufferTexture2D = 0;
#else
PFNGLGENFRAMEBUFFERSPROC             glGenFramebuffers = 0;
PFNGLDELETEFRAMEBUFFERSPROC          glDeleteFramebuffers = 0;
PFNGLBINDFRAMEBUFFERPROC             glBindFramebuffer = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC        glFramebufferTexture2D = 0;
#endif
PFNGLGENRENDERBUFFERSEXTPROC         glGenRenderbuffersEXT = 0;
PFNGLBINDRENDERBUFFEREXTPROC         glBindRenderbufferEXT = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC      glRenderbufferStorageEXT = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC   glCheckFramebufferStatusEXT = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC  glFramebufferRenderbufferEXT = 0;
PFNGLDELETERENDERBUFFERSEXTPROC      glDeleteRenderbuffersEXT = 0;

PFNGLBLENDFUNCSEPARATEPROC           glBlendFuncSeparate = 0;

#ifndef __APPLE__
#ifdef WIN32 
PFNGLACTIVETEXTUREPROC               wglActiveTexture = 0;
#endif
#endif


void LoadExtensions() {
#ifndef __APPLE__
//-------------VBO extensions-----------------------
    glGenBuffers    = (PFNGLGENBUFFERSPROC)
                          SDL_GL_GetProcAddress("glGenBuffersARB");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)
                          SDL_GL_GetProcAddress("glDeleteBuffersARB");
    glBindBuffer    = (PFNGLBINDBUFFERPROC)
                          SDL_GL_GetProcAddress("glBindBufferARB");
    glBufferData    = (PFNGLBUFFERDATAPROC)
                          SDL_GL_GetProcAddress("glBufferDataARB");

    glMapBuffer     = (PFNGLMAPBUFFERPROC)
                          SDL_GL_GetProcAddress("glMapBufferARB");
    glUnmapBuffer   = (PFNGLUNMAPBUFFERPROC)
                          SDL_GL_GetProcAddress("glUnmapBufferARB");
//----------------Attributes--------------------------
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
                                SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
                                    SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)
                                    SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    
    //----------------Shader extensions----------------
    glCreateShader    = (PFNGLCREATESHADERPROC)
                         SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource    = (PFNGLSHADERSOURCEPROC)
                         SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader   = (PFNGLCOMPILESHADERPROC)
                         SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv     = (PFNGLGETSHADERIVPROC)
                         SDL_GL_GetProcAddress("glGetShaderiv");

    
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
                        SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)
                        SDL_GL_GetProcAddress("glDeleteShader");

    glCreateProgram   = (PFNGLCREATEPROGRAMPROC)
                         SDL_GL_GetProcAddress("glCreateProgram");
    glDeleteProgram   = (PFNGLDELETEPROGRAMPROC)
                         SDL_GL_GetProcAddress("glDeleteProgram");
    glAttachShader    = (PFNGLATTACHSHADERPROC)
                         SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram     = (PFNGLLINKPROGRAMPROC)
                         SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv     = (PFNGLGETPROGRAMIVPROC)
                         SDL_GL_GetProcAddress("glGetProgramiv");
    glUseProgram      = (PFNGLUSEPROGRAMPROC)
                         SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)
                            SDL_GL_GetProcAddress("glGetUniformLocation");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)
                            SDL_GL_GetProcAddress("glGetAttribLocation");
    glUniform1f  = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
    glUniform2f  = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
    glUniform3f  = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
    glUniform4f  = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
    glUniform1i  = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
#endif
    glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
    //--------------------------------------------
    #ifdef WIN32
    glGenFramebuffers            = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    glDeleteFramebuffers         = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
    glBindFramebuffer            = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D       = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    #else   
    glGenFramebuffers            = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
    glDeleteFramebuffers         = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
    glBindFramebuffer            = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D       = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
    #endif
    glGenRenderbuffersEXT        = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
    glBindRenderbufferEXT        = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
    glRenderbufferStorageEXT     = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
    glCheckFramebufferStatusEXT  = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
    glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
    glDeleteRenderbuffersEXT     = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");

#ifndef __APPLE__
    //--------------------------------------------
    #ifdef WIN32
    wglActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");
    #endif
  #endif
}
//-----------------------------------------
void UnloadExtensions() {
#ifndef __APPLE__
    glGenBuffers    = 0;
    glDeleteBuffers = 0;
    glBindBuffer    = 0;
    glBufferData    = 0;
    glMapBuffer     = 0;
    glUnmapBuffer   = 0;


    glCreateShader       = 0;
    glShaderSource       = 0;
    glCompileShader      = 0;
    glGetShaderiv        = 0;
    glCreateProgram      = 0;
    glAttachShader       = 0;
    glLinkProgram        = 0;
    glGetProgramiv       = 0;
    glUseProgram         = 0;
    glGetUniformLocation = 0;
    glUniform1f          = 0;
    glUniform2f          = 0;
    glUniform3f          = 0;
    glUniform4f          = 0;
    glUniform1i          = 0;
    glGetInfoLogARB      = 0;

#endif
    glGenFramebuffers = 0;
    glGenRenderbuffersEXT = 0;
    glDeleteFramebuffers = 0;
    glBindFramebuffer = 0;
    glBindRenderbufferEXT = 0;
    glFramebufferTexture2D = 0;
    glRenderbufferStorageEXT = 0;
    glCheckFramebufferStatusEXT = 0;
    glFramebufferRenderbufferEXT = 0;
    glDeleteRenderbuffersEXT = 0;

#ifndef __APPLE__
#ifdef WIN32 
    wglActiveTexture = 0;
#endif
#endif
}

//-------------------------------
bool hasExtension(const char * name){
    if (!GLExtensionString)
        GLExtensionString = glGetString(GL_EXTENSIONS);

    return strstr((const char*)GLExtensionString, name);
}

