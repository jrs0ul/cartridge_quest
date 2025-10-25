
#include "SysConfig.h"
#include <cwchar>
#include <cstdio>

#ifndef __ANDROID__
#include "Xml.h"


    bool SystemConfig::load(const char * config){

        Xml conf;


        if (!conf.load(config))
            return false;

        XmlNode * settings = 0;
        settings = conf.root.getNode(L"Settings");
        if (settings){

            wchar_t tmp[255];
            char ctmp[255];

            XmlNode * nod = 0;
            nod = settings->getNode(L"MusicVolume");
            if (nod){
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                musicVolume = atof(ctmp);
            }

            nod = 0;
            nod = settings->getNode(L"Renderer");
            if (nod)
            {
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                renderIdx = atoi(ctmp);
            }


            nod = 0;
            nod = settings->getNode(L"ScreenWidth");
            if (nod){
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                ScreenWidth = atoi(ctmp);
            }

            nod = 0;
            nod = settings->getNode(L"ScreenHeight");
            if (nod){
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                ScreenHeight = atoi(ctmp);
            }

            nod = 0;
            nod = settings->getNode(L"isWindowed");
            if (nod){
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                useWindowed = atoi(ctmp);
            }

            nod = 0;
            nod = settings->getNode(L"screenScaleX");
            if (nod)
            {
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                screenScaleX = atoi(ctmp);
            }

            nod = 0;
            nod = settings->getNode(L"screenScaleY");
            if (nod)
            {
                wcscpy(tmp, nod->getValue());
                wcstombs(ctmp, tmp, 100);
                screenScaleY = atoi(ctmp);
            }




        }

        conf.destroy();

        return true;
    }
//------------------------------------------
bool SystemConfig::write(const char * config)
{

    printf("writing: %s\n", config);

        char buf[255];
        wchar_t wbuf[255];
        Xml conf;

        XmlNode Settings;
        Settings.setName(L"Settings");
        Settings.setValue(L"\n");

        XmlNode _width;
        sprintf(buf, "%f", musicVolume);
        printf("%s\n", buf);
        mbstowcs(wbuf, buf, 255);
        _width.setName(L"MusicVolume");
        _width.setValue(wbuf);
        Settings.addChild(_width);

        XmlNode _Renderer;
        sprintf(buf, "%d", renderIdx);
        mbstowcs(wbuf, buf, 255);
        _Renderer.setName(L"Renderer");
        _Renderer.setValue(wbuf);
        Settings.addChild(_Renderer);

        XmlNode _ScreenWidth;
        sprintf(buf, "%d", ScreenWidth);
        mbstowcs(wbuf, buf, 255);
        _ScreenWidth.setName(L"ScreenWidth");
        _ScreenWidth.setValue(wbuf);
        Settings.addChild(_ScreenWidth);

        XmlNode _ScreenHeight;
        sprintf(buf, "%d", ScreenHeight);
        mbstowcs(wbuf, buf, 255);
        _ScreenHeight.setName(L"ScreenHeight");
        _ScreenHeight.setValue(wbuf);
        Settings.addChild(_ScreenHeight);

        XmlNode _isWindowed;
        sprintf(buf, "%d", useWindowed);
        mbstowcs(wbuf, buf, 255);
        _isWindowed.setName(L"isWindowed");
        _isWindowed.setValue(wbuf);
        Settings.addChild(_isWindowed);

        XmlNode _screenScaleX;
        sprintf(buf, "%d", screenScaleX);
        mbstowcs(wbuf, buf, 255);
        _screenScaleX.setName(L"screenScaleX");
        _screenScaleX.setValue(wbuf);
        Settings.addChild(_screenScaleX);

        XmlNode _screenScaleY;
        sprintf(buf, "%d", screenScaleY);
        mbstowcs(wbuf, buf, 255);
        _screenScaleY.setName(L"screenScaleY");
        _screenScaleY.setValue(wbuf);
        Settings.addChild(_screenScaleY);




       conf.root.addChild(Settings);


        if (!conf.write(config))
        {
            conf.destroy();
            return false;
        }
        conf.destroy();
        return true;
    }
#endif
