#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "BasicUiControl.h"
#include "../Colors.h"

class SpriteBatcher;

class Button: public BasicControl{

    
 public:
    int height;
    int width;

    bool pressed;
    COLOR c;
    float scalex;
    int state;

    
    Button(int dx = 0,
            int dy = 0,
            int w = 50,
            int h = 50,
            COLOR _c = COLOR(1,1,1,1)){
        setpos(dx, dy);
        width = w; height = h;
        pressed = false;
        c = _c;
        scalex = 1.0f;
        
    }
    
    void init(int dx = 0,
             int dy = 0,
             int w = 50,
             int h = 50,
             COLOR _c = COLOR(1,1,1,1)){
        setpos(dx, dy);
        width = w; height = h;
        pressed = false;
        c = _c;
        
    }
    void draw(SpriteBatcher& pics,
              unsigned picindex, unsigned frame = 0);
    void drawTextnPicture(SpriteBatcher& pics, unsigned frame, unsigned index, unsigned font,
                          const char* text);
    bool isPointerOnTop(int px, int py);
    void set(int dx, int dy,
             int w, int h, COLOR _c = COLOR(1,1,1,1)){ setpos(dx, dy);
                                                       width = w; height = h; c = _c;}
    void shiftstate();
    void reset(){pressed = false; scalex = 1.0f; state = 0;}
};
#endif //_BUTTON_H_
