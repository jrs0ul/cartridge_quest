#include "Button.h"
#include "../SpriteBatcher.h"
#include "Text.h"


void Button::draw(SpriteBatcher& pics,
                  unsigned picindex ,
                  unsigned frame){
    PicData* d = pics.getInfo(picindex);
    pics.draw(picindex, getX() + width / 2, getY() + height / 2,
              frame, true, 
              (width/(d->twidth*1.0f)) * scalex,
              height/(d->theight*1.0f), 0.0f, c , c);
}
//----------------------------------

void  Button::drawTextnPicture(SpriteBatcher& pics,
                               unsigned index,
                               unsigned frame,
                               unsigned font,
                               const char* text)
{
    PicData* d = pics.getInfo(index);
    pics.draw(index, getX() + width / 2, getY() + height / 2,
              frame, true, 
              width/(d->twidth * 1.0f), height/(d->theight * 1.0f), 0.0f, c , c);
    float textlen = strlen(text) * 9.2f;
  
    WriteShadedText( getX() + width/2.0f - textlen/2.0f, getY() + height/2 - 8, pics, font, text, 1.0f, 1.0f, COLOR(1,1,1));
    
}
//---------------------------------


bool Button::isPointerOnTop(int px, int py){
    if ((px > getX()) && (px < getX() + width)
            && (py > getY()) && (py < getY() + height))

        return true;

    return false;
}
//---------------------------------
void Button::shiftstate(){
     if (pressed) pressed = false;
     else 
         pressed = true;
}


