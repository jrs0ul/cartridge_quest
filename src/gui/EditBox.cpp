#include "EditBox.h"
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
    #ifdef _MSC_VER
        #include <SDL_scancode.h>
    #else
        #include <SDL2/SDL_scancode.h>
    #endif
#else
    #ifndef ANDROID
        #include <SDL2/SDL_scancode.h>
    #endif
#endif

#include "Text.h"




void EditBox::init(unsigned int dx, unsigned int dy, const char* dtitl, unsigned char dmaxl)
{
    setpos(dx, dy);
    strcpy(title,dtitl);
    maxlength=dmaxl;
    entered=false;
    canceled=false;
    strcpy(text,"");
#ifndef ANDROID
    pressedKey = SDL_SCANCODE_RETURN;
#endif
}
//------------------------------------
void EditBox::getInput(const char* eventText, unsigned keydown)
{
#ifndef ANDROID
    if (pressedKey != keydown)
    {
        if ( keydown == SDL_SCANCODE_RETURN)
        {
            entered = true;
        }
        else if ( keydown == SDL_SCANCODE_ESCAPE )
        {
            canceled = true;
        }
        else if (keydown == SDL_SCANCODE_BACKSPACE)
        {
            if (strlen(text))
            {
                text[strlen(text) - 1] = 0;
            }
        }
        else if (strlen(text) < 39 && keydown != 0)
        {
            strcat(text, eventText);
        }
    }

    pressedKey = keydown;
#endif
}
//------------------------------------
void EditBox::draw(SpriteBatcher& pics, unsigned font)
{

    WriteShadedText(getX() + 2, getY() + 1, pics, font, title);

    if ((rand() % 100) % 5 == 0)
    {
        char tmp[80];
        sprintf(tmp,"%s_",text);   
        WriteShadedText(getX() + 5, getY() + 16, pics, font,tmp);
    }
    else
    {
        WriteShadedText(getX() + 5, getY() + 16, pics, font,text);
    }

}
//----------------------------------------------
void EditBox::reset()
{
    strcpy(text,"");
    entered = false;
    canceled = false;
#ifndef ANDROID
    pressedKey = SDL_SCANCODE_RETURN;
#endif
}

