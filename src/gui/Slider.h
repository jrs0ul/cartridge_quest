#pragma once

#include "BasicUiControl.h"

class SpriteBatcher;

class ScroollControl:public BasicControl
{
    public:
        char title[30];
        bool selected;
        bool canceled;
        unsigned char pressedkey;
        long state;
        long maxstate;
        int step;

        void init(unsigned int dx, unsigned int dy, const char* dt, long defstate, long dmaxstate, int dstep);
        void reset();
        void getInput(const unsigned char* keys, const unsigned char* oldKeys);
        void draw(SpriteBatcher& pics, unsigned rod, unsigned bg,  unsigned font);
};

