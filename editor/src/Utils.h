
#ifndef _UTILS_H
#define _UTILS_H


#include "../../src/SpriteBatcher.h"





struct _POINT{
    int x; int y;
};



int     _round(double x);

void    DrawText(int x, int y,const char* text, SpriteBatcher& pic, int imgindex,
                float scale=1.0f,
                COLOR color=COLOR(1.0f, 1.0f, 1.0f, 1.0f));

void    DrawBlock(SpriteBatcher& pics, int x, int y, int width, int height,
                  COLOR c = COLOR(0.0f, 0.0f, 0.0f, 1.0f), COLOR c2 = COLOR(0,0,0,0));

//bool    CirclesColide(float x1,float y1,float radius1, float x2, float y2, float radius2);

//_POINT* Line(int x1, int y1, int x2, int y2, int& count, int gridw);

#endif //UTILS_H
