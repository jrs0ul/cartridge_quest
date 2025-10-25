#ifndef _SYSTEMCONFIG_H
#define _SYSTEMCONFIG_H

#include <cstring>

    class SystemConfig{


    public:
        float musicVolume;

        int ScreenWidth;
        int ScreenHeight;
        int screenScaleX;
        int screenScaleY;
        int useWindowed;
        int soundFXVolume;
        int renderIdx;


        SystemConfig(){
            renderIdx = 0; //OPENGL
            musicVolume = 0.2f;

            useWindowed = false;
            screenScaleX = 3.f;
            screenScaleY = 3.f;
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
            ScreenWidth = 640;
            ScreenHeight = 360;
#else
            ScreenWidth = 640;
            ScreenHeight = 360;

#endif

        }

#ifndef __ANDROID__
        bool load(const char * config);
        bool write(const char * config);
#endif
    };



#endif //SYSTEMCONFIG_H
