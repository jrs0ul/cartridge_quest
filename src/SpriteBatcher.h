/*
 The Disarray 
 by jrs0ul(jrs0ul ^at^ gmail ^dot^ com) 2025
 -------------------------------------------
 Sprite batcher
 mod. 2025.09.28
 */
#pragma once

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


#include "Image.h"
#include "DArray.h"
#include "Colors.h"
#include "ShaderProgram.h"

struct Sprite
{
    char name[128];
    int startX;
    int startY;
    int width;
    int height;
};


struct PicData{
    char name[256];
    int twidth;
    int theight;
    int width;
    int height;
    int filter;

    DArray<Sprite> sprites;


    //additional data for faster rendering
    float htilew;
    float htileh;
    int vframes;
    int hframes;
    float partX;
    float partY;
};
//-------------------------
struct SpriteBatchItem{
    float x;
    float y;
    long textureIndex;
    unsigned int frame;
    bool useCenter;
    float scaleX ;
    float scaleY ;
    float rotationAngle;
    COLOR upColor[2];
    COLOR dwColor[2];

    SpriteBatchItem(){
        x = 0;
        y = 0;
        textureIndex = 0;
        frame = 0;
        useCenter = false;
        scaleX = 1.0f;
        scaleY = 1.0f ;
        rotationAngle = 0.0f;
        upColor[0] = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
        upColor[1] = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
        dwColor[0] = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
        dwColor[1] = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    }
};
//==================================

class PicsContainer{
    DArray<GLuint> TexNames;
    DArray<PicData> PicInfo;
    DArray<SpriteBatchItem> batch;
public:
    void drawVA(void * vertices, void * uvs, void *colors,
                unsigned uvsCount, unsigned vertexCount,
                ShaderProgram* shader,
                bool useVulkan,
                VkCommandBuffer* vkCmd = nullptr,
                VkDevice* vkDevice = nullptr);
private:
    void resizeContainer(unsigned long index,
                         int twidth, int theight, int filter,
                         const char * name,
                         bool createTextures = true,
                         GLuint texname = 0);

public:
    PicsContainer(){}
#ifndef __ANDROID__
    bool initContainer(const char* list);
#else
    bool initContainer(const char* list, AAssetManager* assman);
#endif

    //load textures from list in textfile
#ifndef __ANDROID__
    bool load(const char* list);
#else
    bool load(const char* list, AAssetManager* assman);
#endif


    void destroy();

    //adds sprite to batch
    void draw( long textureIndex,
                float x, float y,
                unsigned int frame = 0,
                bool useCenter = false,
                float scaleX = 1.0f, float scaleY = 1.0f,
                float rotationAngle = 0.0f,
                COLOR upColor = COLOR(1.0f, 1.0f, 1.0f, 1.0f),
                COLOR dwColor = COLOR(1.0f, 1.0f, 1.0f, 1.0f),
                bool flipColors = false
               );
    //draws all sprites in batch
    void drawBatch(ShaderProgram * justColor,
                   ShaderProgram * uvColor,
                   int method = 0,
                   bool useVulkan = false,
                   VkCommandBuffer* vkCmd = nullptr,
                   VkDevice* vkDevice = nullptr);

    GLuint getname(unsigned long index);
    unsigned getTextureCount(){return TexNames.count();}
    PicData* getInfo(unsigned long index);
    unsigned long count(){return PicInfo.count();}
    int findByName(const char* picname, bool debug = false);
    //to make textures from images loaded in defferent threads
    void makeTexture(Image& img, 
                     const char * name,
                     unsigned long index,
                     int twidth, int theight, int filter=0);
#ifndef __ANDROID__
    bool loadFile(const char* file, unsigned long index,
                             int twidth, int theight, int filter);
#else
    bool loadFile(const char* file, unsigned long index,
                                 int twidth, int theight, int filter,
                                 AAssetManager* man);
#endif

   
    //loads images using special array loaded from file
    bool loadFile(unsigned long index, const char * BasePath);
    //attach already created texture
    void attachTexture(GLuint textureID, unsigned long index,
                       int width, int height,
                       int twidth, int theight, int filter = 0);
    void remove(unsigned long index);

};


