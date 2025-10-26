#ifndef DECAL_H
#define DECAL_H


#include "Colors.h"

class SpriteBatcher;

class Decal
{
public:
    COLOR color;
    float x;
    float y;
    int frame;

    Decal() :
    color(1.f, 1.f, 1.f, 1.f)
    {
        x = y = 0.f;
        frame = 0;
    }
    void draw(SpriteBatcher& pics,
              unsigned index,
              float posx, float posy,
              int ScreenWidth, int ScreenHeight);
};


#endif //DECAL_H
