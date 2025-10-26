#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "Button.h"


class ScrollBar: public BasicControl{
    
    Button prevPage;
    Button nextPage;
    
    unsigned pageSize;
    unsigned size;
    unsigned _state;
    
    bool isVertical;
    int elementSize;
public:
    
    ScrollBar(){ size = 1; _state = 0; pageSize = 1; elementSize = 1;}
    void set(int x, int y, unsigned pSize, unsigned newsize, unsigned defaultState,
             int eSize = 1, bool isItVertical = false);
    void processInput(int px, int py);
    bool isPointerOnPrevpage(int px, int py);
    bool isPointerOnNextpage(int px, int py);
    void draw(SpriteBatcher& pics, unsigned long buttons, unsigned prev, unsigned next);
    unsigned state(){return _state;}
    void setSize(unsigned _siz);
    void pageDown();
    void pageUp();
};



#endif //SCROLLBAR_H
