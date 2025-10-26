#include <cstring>
#include "Slider.h"
#include "Text.h"
#include "../SpriteBatcher.h"


void ScroollControl::init(unsigned int dx, unsigned int dy,
        const char *dt, long defstate, long dmaxstate, int dstep)
{

    setpos(dx, dy);
    strcpy(title,dt);
    state = defstate;
    maxstate = dmaxstate;
    step = dstep;
    selected = false;
    canceled = false;
    deactivate();
    pressedkey = 0;

}

void ScroollControl::draw(SpriteBatcher& pics, unsigned rod, unsigned bg, unsigned font)
{
    WriteText(getX(), getY(), pics, font, title);

    for (int i = 0; i < maxstate / step; i++)
    {
        pics.draw(bg, getX() + 10+i, getY() + 16, 2);
    }

    pics.draw(rod, getX() + 10 + state/step, getY() + 16+8, 1, true, 1.0f,1.0f, 0.0f, 
              COLOR(1.0f,1.0f,1.0f, 1.f), COLOR(1.f,1.f,1.f,1.f));

}

void ScroollControl::getInput(const unsigned char* keys, const unsigned char* oldKeys)
{

    if (keys[4] && !oldKeys[4]) 
    {
        selected = true;      //enter
    }

    if (keys[5] && !oldKeys[5])
    {
        canceled = true;      //esc
    }

    if (keys[3])
    {
        if (state>step) //up
            state-=step;
    }
    else if (keys[2])
    {
        if (state<maxstate-step)  //down
            state+=step;
    }


}
//----------------------------
void ScroollControl::reset()
{
    selected=false;
    canceled=false;
}
