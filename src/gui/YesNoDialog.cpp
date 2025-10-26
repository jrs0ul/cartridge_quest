
#include "YesNoDialog.h"
#include "Text.h"

void YesNoDialog::set(int _x, int _y, int w, int h){
    setpos(_x, _y);
    width = w;
    height = h;
    yes.init(_x + 10, _y + h - 60, 60, 50);
    no.init(_x + w - 70, _y + h - 60, 60, 50);
}
//-----------------------
void YesNoDialog::draw(SpriteBatcher& pics, unsigned font, unsigned decorations,
          unsigned buttons, unsigned buttonframes, float decorationSize){
    COLOR bgC(0.5f, 0.5f, 0.5f, 0.5f);
    COLOR bgColor(0.4f, 0.4f, 0.4f, 0.9f);
    pics.draw(-1, 0, 0, 0, false, 480, 320, 0, bgC, bgC);
    pics.draw(-1, getX() + 32, getY(), 0, false, width - 64, height, 0, bgColor, bgColor);
    pics.draw(decorations, getX(), getY(), 0, false, 32/64.0f, height/decorationSize, 0, bgColor, bgColor);
    pics.draw(decorations, getX() + width - 32, getY(), 1, false, 32/64.0f, height/decorationSize, 0, bgColor, bgColor);
    
    WriteShadedText(getX() + 10, getY() + 20, pics, font, "Do you want to reset ?");
    yes.drawTextnPicture(pics, buttonframes, buttons, font, "Yes");
    no.drawTextnPicture(pics, buttonframes, buttons, font, "No");
}
//-----------------------
bool YesNoDialog::isPointerOnYes(int px, int py){
    return yes.isPointerOnTop(px, py);
}
//-----------------------
bool YesNoDialog::isPointerOnNo(int px, int py){
    return no.isPointerOnTop(px, py);
}
