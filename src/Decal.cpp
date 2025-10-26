#include "Decal.h"
#include "Usefull.h"
#include "SpriteBatcher.h"
#include "Consts.h"

void Decal::draw(SpriteBatcher& pics,
                 unsigned index,
                 float posx, float posy,
                 int ScreenWidth, int ScreenHeight)
{

    const float decalX = x + posx;
    const float decalY = y + posy;

    if (decalX + HALF_TILE_WIDTH < 0 || decalX - HALF_TILE_WIDTH > ScreenWidth ||
        decalY + HALF_TILE_WIDTH < 0 || decalY - HALF_TILE_WIDTH > ScreenHeight)
    {
        return;
    }



    pics.draw(index,
              decalX,
              decalY,
              frame,
              true,
              1.0f, 1.0f, 0.f,
              color,
              color); 
}
