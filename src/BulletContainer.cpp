#include "BulletContainer.h"
#include <cstring>
#include "Consts.h"
#include "bullet.h"
#include "SpriteBatcher.h"

void CBulletContainer::add(Bullet& newbulet)
{
    buls.add(newbulet);
}
//---------------------------
void CBulletContainer::removeDead()
{
    if (!buls.count())
    {
        return;
    }

    for (int i = buls.count() - 1; i >= 0; --i)
    {

        if (!buls[i].exists)
        {
            buls.remove(i);
        }
    }
}
//---------------------------

void CBulletContainer::draw(SpriteBatcher& pics, float posx, float posy, int ScreenWidth, int ScreenHeight)
{
    for (unsigned z = 0; z < buls.count(); z++)
    {
        const float bulletX = buls[z].x + posx;
        const float bulletY = buls[z].y + posy;

        if (bulletX + HALF_TILE_WIDTH < 0 || bulletX - HALF_TILE_WIDTH > ScreenWidth ||
            bulletY + HALF_TILE_WIDTH < 0 || bulletY - HALF_TILE_WIDTH > ScreenHeight)
        {
            continue;
        }


        pics.draw(6,
                bulletX,
                bulletY,
                buls[z].frame,
                true,
                1.0f,
                1.0f,
                (buls[z].angle + (M_PI / 2.0f)) * (180 / M_PI)
                ); 
    }

}
//---------------------------
void CBulletContainer::update(const bool** colisionGrid, DArray<Dude>& dudes, int mapWidth, int mapHeight)
{
    for (unsigned i = 0; i < buls.count(); i++)
    {
        buls[i].update(colisionGrid, mapWidth, mapHeight);

        if (buls[i].onHit(dudes))
        {
            if (!buls[i].explode)
            {
                buls[i].explode = true;
                buls[i].frame = (buls[i].type == WEAPONTYPE_SHRINKER) ? BF_DUKE_SHRINKER_EXPLODE : BF_YELLOW_PEW_EXPLODE;
            }
        }

        removeDead();
    }
}

//---------------------------

void CBulletContainer::destroy()
{
    buls.destroy();
}
