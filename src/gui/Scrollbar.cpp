
#include "Scrollbar.h"

void ScrollBar::set(int x, int y, unsigned pSize, unsigned newsize, unsigned defaultState,
                    int eSize, bool isItVertical){
    
    pageSize = pSize;
    size = newsize;
    _state = defaultState;
    elementSize = eSize;
    isVertical = isItVertical;
    setpos(x, y);
    prevPage.set(x, y, 40, 40);
    nextPage.set(x + ((isVertical) ? 0 : size * elementSize + 40), 
                 y + ((isVertical) ? size * elementSize + 40: 0),
                 40, 40);
}
    
//-------------------
void ScrollBar::setSize(unsigned _siz){
    size = _siz;
    prevPage.set(getX(), getY(), 40, 40);
    nextPage.set(getX() + ((isVertical) ? 0 : size * elementSize + 40), 
                 getY() + ((isVertical) ? size * elementSize + 40 : 0),
                 40, 40);
}

//--------------------
void ScrollBar::processInput(int px, int py){
    int barSize = (int)(elementSize * size);
    int minX = getX() + ((isVertical)? 0 : 40);
    int maxX = getX() + ((isVertical)? 40 : barSize + 40);
    int minY = getY() + ((isVertical)? 40 : 0);
    int maxY = getY() + ((isVertical)? barSize + 40 : 40);
    if ((px > minX) && ( px < maxX) &&
        (py > minY) && ( py < maxY)){
        int tmpState = (isVertical ? (py - getY() - 40)/ (int)elementSize :
                                     (px - getX() - 40)/ (int)elementSize )
                        - ((int)pageSize / 2);
        if (tmpState < 0)
            tmpState = 0;
        if (tmpState  > (int)(size - pageSize))
            tmpState = (int)(size - pageSize);
        _state = (unsigned)tmpState;
    
    }
}
//--------------------
bool ScrollBar::isPointerOnPrevpage(int px, int py){
    return prevPage.isPointerOnTop(px, py);
}
//---------------------
bool ScrollBar::isPointerOnNextpage(int px, int py){
    return nextPage.isPointerOnTop(px, py);
}
//----------------------
void ScrollBar::draw(SpriteBatcher& pics, unsigned long buttons, unsigned prev, unsigned next){
    pics.draw(-1, getX() + ((isVertical) ? 10 : 40), getY() + ((isVertical) ? 40 : 10), 0, false,
              ((isVertical) ? 20 : size * elementSize),
              ((isVertical) ? size * elementSize : 20),
              0, COLOR(0.4f, 0.4f, 0.4f, 0.8f), COLOR(0.4f, 0.4f, 0.4f, 0.8f)
              );
    pics.draw(buttons,
              ((isVertical) ? getX()+5 : getX() + 40 +_state * elementSize),
              ((isVertical) ? getY() + 40 + _state * elementSize : getY() + 5),
              7, false,
              ((isVertical) ? 30 : elementSize * pageSize )/48.0f,
              ((isVertical) ? elementSize * pageSize : 30 )/48.0f,
              0, COLOR(0.6f, 0.6f, 0.6f), COLOR(0.6f, 0.6f, 0.6f)
              
              );
    prevPage.draw(pics, buttons, prev);
    nextPage.draw(pics, buttons, next);
}
//------------
void ScrollBar::pageDown(){
    if (_state + pageSize < size - pageSize)
        _state += pageSize;
    else
        _state = size - pageSize;
}
//-------------
void ScrollBar::pageUp(){
    if ((int)(_state - pageSize) > 0)
        _state -= pageSize;
    else 
        _state = 0;

}


