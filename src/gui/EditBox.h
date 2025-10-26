#ifndef _EDITBOX_H_
#define _EDITBOX_H_

#include "BasicUiControl.h"

class SpriteBatcher;

class EditBox : public BasicControl
{
public:
        unsigned char maxlength;
        char text[40];
        bool entered;
        bool canceled;
        char title[20];
        unsigned char pressedKey;
        void init(unsigned int dx, unsigned int dy, const char* dtitl, unsigned char dmaxl);
        void getInput(const char* eventText, unsigned keydown);
        void draw(SpriteBatcher& pics, unsigned font);
        void reset();
};

#endif //_EDITBOX_H_
