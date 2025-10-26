#include "Dude.h"

#include <cmath>
#include "Usefull.h"
#include "map.h"
#include "gui/Text.h"
#include "bullet.h"
#include "Item.h"
#include "BulletContainer.h"
//


//------------------------
void Dude::heal()
{
    if (hp < 100)
    {
        if (hp > 90)
        {
            hp = 100;
        }
        else
        {
            hp += 10;
        }
    }
}
//-------------------------
void Dude::initMonsterHP()
{
    hp = MONSTER_BASE_HP + rand()%10;
}
//--------------------------------
bool Dude::damage(int dmg)
{
    hp -= dmg;
    return hp <= 0;
}
//-------------------------------------
bool Dude::isColideWithOthers( DArray<Dude>& chars, float newx, float newy, bool coop, unsigned monsterCount)
{

    for (unsigned i = 0; i < chars.count(); ++i)
    {
        if (chars[i].id == id)
        {
            continue;
        }

        if (chars[i].spawn)
        {
            continue;
        }

        if (chars[i].shot)
        {
            if (!(coop && i >= monsterCount))
            {
                continue;
            }
        }

        if (!shrinked && chars[i].shrinked)
        {
            continue;
        }

        if (CirclesColide(newx, newy, 10.0f, chars[i].x, chars[i].y, 10.0f))
        {
            return true;
        }
    }

    return false;
}
//--------------------------------------
void Dude::respawn()
{

    stim++;

    if (stim % 5==0)
    {
        if (frame == skinCount * 4 + 2) 
        {
            frame = skinCount * 4 + 3;
        }
        else 
        {
            frame = skinCount * 4 + 2;
        }
    }

    if (stim >= 80) 
    {
        spawn = false;
        stim = 0;
        frame = (activeSkin[currentWeapon] + 1) * 4 - 2;
    }
}
//--------------------------------------------------
void Dude::update()
{
    Vector3D old = Vector3D(x, y, angle);

    for (int i = 1; i < ENTITY_POSITION_HISTORY_LEN; ++i)
    {
        oldPos[i - 1] = oldPos[i];
    }


    oldPos[ENTITY_POSITION_HISTORY_LEN - 1] = old;

    if (shrinked)
    {
        shrinkTimer -= SHRINKED_TIMER_STEP;

        if (shrinkTimer <= 0)
        {
            shrinked = false;
        }
    }

    ps.updateSystem();
}
//--------------------------------------------------
void Dude::rotate(float ang)
{
    angle += ang;

    if (angle >= 6.28f)
    {
        angle = 0;
    }

}


//----------------------------------------------------
//creature moves using the rotation angle
//returns false if the creature can't move in X or Y axis direction
bool Dude::move(float walkSpeed,
                float strifeSpeed,
                float radius,
                CMap& map,
                bool isCoop)
{

    Vector3D vl = MakeVector((shrinked ? walkSpeed * 0.3f : walkSpeed), (shrinked ? strifeSpeed * 0.3f : strifeSpeed), angle);
    return movement(vl, radius, (const bool**)map._colide, map.width(), map.height(), map.mons, isCoop, map.enemyCount);
}
//-----------------------------------------------------
bool Dude::moveGamePad(const Vector3D& movementDir,
                        float radius,
                        CMap& map,
                        bool isCoop)
{
    return movement(movementDir, radius, (const bool**)map._colide, map.width(), map.height(), map.mons, isCoop, map.enemyCount);
}


//----------------------------------------------------
bool Dude::movement(Vector3D dir,
                    float radius,
                    const bool** map,
                    int mapsizex,
                    int mapsizey,
                    DArray<Dude>& chars,
                    bool coop,
                    unsigned monsterCount)
{

    float difx = dir.x;
    float dify = dir.y;

    float newposx = x + difx;
    float newposy = y - dify;


    Vector3D p1; 
    Vector3D p2;
    Vector3D p3;
    Vector3D p4;

    Vector3D p5; 
    Vector3D p6;
    Vector3D p7;
    Vector3D p8;

    radius *= (shrinked) ? 0.3f : 1.f;

    p1.x = round((newposx - radius) / 32.0f);

    if (p1.x > mapsizex-1)
    {
        p1.x = mapsizex-1;
    }

    p1.y = round((y - radius) / 32.0f);

    if (p1.y > mapsizey-1)
    {
        p1.y = mapsizey-1;
    }

    p2.x=round((newposx+radius)/32.0f);

    if (p2.x>mapsizex-1)
    {
        p2.x=mapsizex-1;
    }

    p2.y=p1.y;

    p3.x=p1.x;

    p3.y=round((y+radius)/32.0f);

    if (p3.y>mapsizey-1)
    {
        p3.y=mapsizey-1;
    }

    p4.x=p2.x;
    p4.y=p3.y;

//--------

   
    p5.x=round((x-radius)/32.0f);
    if (p5.x>mapsizex-1)
        p5.x=mapsizex-1;
    p5.y=round((newposy-radius)/32.0f);
    if (p5.y>mapsizey-1)
        p5.y=mapsizey-1;

    p6.x=round((x+radius)/32.0f);
    if (p6.x>mapsizex-1)
        p6.x=mapsizex-1;
    p6.y=p5.y;

    p7.x=p5.x;
    p7.y=round((newposy+radius)/32.0f);
    if (p7.y>mapsizey-1)
        p7.y=mapsizey-1;

    p8.x=p6.x;
    p8.y=p7.y;



    int dirx = 0;
    int diry = 0;


    if ((!map[(int)p1.y][(int)p1.x]) &&
        (!map[(int)p2.y][(int)p2.x]) &&
        (!map[(int)p3.y][(int)p3.x]) &&
        (!map[(int)p4.y][(int)p4.x]) && 
        (!isColideWithOthers(chars, newposx, y, coop, monsterCount)) &&
        ((newposx)<=(mapsizex-1) * TILE_WIDTH) &&
        ((newposy)<((mapsizey-1) * TILE_WIDTH)) &&
        ((newposx-radius)>=-radius) &&
        ((y-radius)>=-radius)

       )
    {
        if (x > newposx)
        {
            dirx = 1;
        }
        else if (x < newposx)
        {
            dirx = 2;
        }

        x = x + difx;

    }

    if ((!map[(int)p5.y][(int)p5.x]) &&
        (!map[(int)p6.y][(int)p6.x]) &&
        (!map[(int)p7.y][(int)p7.x]) &&
        (!map[(int)p8.y][(int)p8.x]) &&
        (!isColideWithOthers(chars, x, newposy, coop, monsterCount)) &&
        ((newposx)<=(mapsizex-1) * TILE_WIDTH) &&
        ((newposy)<((mapsizey-1) * TILE_WIDTH))&&
        ((newposx-radius)>=-radius) &&
        ((newposy-radius)>=-radius))
    {

        if (y > newposy)
        {
            diry = 1;
        }
        else if (y < newposy)
        {
            diry = 2;
        }

        y = y - dify;
    }

    if ((diry > 0) || (dirx > 0))
    {

        tim++;

        if (tim >= 30)
        {
            tim = 0;
        }

        frame = tim / 10 + activeSkin[currentWeapon] * 4;
    }

    if ((diry == 0) || (dirx == 0))
    {
        return false;
    }

    return true;


}
//--------------------------------------------------------------
void Dude::draw(SpriteBatcher& pics, unsigned index, float posx, float posy, int ScreenWidth, int ScreenHeight)
{

    const float scale = (shrinked) ? 0.3f : 1.f;

    if (equipedGame == ITEM_GAME_SPEEDBALL)
    {
        for (int i = 0; i < ENTITY_POSITION_HISTORY_LEN; ++i)
        {
            if (i % 2 == 0)
            {
                continue;
            }

            const float olddudex = oldPos[i].x + posx;
            const float olddudey = oldPos[i].y + posy;
            const float oldAngle = oldPos[i].z;

            if (olddudex + HALF_TILE_WIDTH < 0 || olddudex - HALF_TILE_WIDTH > ScreenWidth ||
                    olddudey + HALF_TILE_WIDTH < 0 || olddudey - HALF_TILE_WIDTH > ScreenHeight)
            {
                continue;
            }

            const float alpha = (i * (0.5f / ENTITY_POSITION_HISTORY_LEN)) + 0.1f;

            pics.draw(index,
                    olddudex,
                    olddudey,
                    frame,
                    true,
                    scale,
                    scale,
                    (oldAngle + M_PI / 2.f) * (180 / M_PI),
                    COLOR(0.5f, 0.5f, 1.f, alpha),
                    COLOR(0.5f, 0.5f, 1.f, alpha));

        }
    }



    const float dudex = x + posx;
    const float dudey = y + posy;

    if (dudex + HALF_TILE_WIDTH < 0 || dudex - HALF_TILE_WIDTH > ScreenWidth ||
        dudey + HALF_TILE_WIDTH < 0 || dudey - HALF_TILE_WIDTH > ScreenHeight)
    {
        return;
    }

    pics.draw(index,
              dudex,
              dudey,
              frame,
              true,
              scale,
              scale,
              (angle + M_PI / 2.f) * (180 / M_PI),
              COLOR(r,g,b, 1.f),
              COLOR(r,g,b, 1.f));


}
//---------------------------------------------
void Dude::drawParticles(SpriteBatcher& pics, float posx, float posy, int ScreenWidth, int ScreenHeight)
{
    const float dudex = x + posx;
    const float dudey = y + posy;

    if (dudex + HALF_TILE_WIDTH < 0 || dudex - HALF_TILE_WIDTH > ScreenWidth ||
        dudey + HALF_TILE_WIDTH < 0 || dudey - HALF_TILE_WIDTH > ScreenHeight)
    {
        return;
    }

    ps.drawParticles(pics, 15, Vector3D(dudex, 0, dudey));
}

//----------------------------------------------------------
bool Dude::shoot(bool useBullets, WeaponTypes weaponType, CBulletContainer* bulcon)
{

    if ((useBullets) && (ammo<=0))
    {
        return false;
    }


    canAtack = false;
    frame = (activeSkin[currentWeapon] + 1) * 4 - 1;

    if (useBullets)
    {
        if (ammo <= 0)
        {
            return false;
        }


        switch(weaponType)
        {
            case WEAPONTYPE_REGULAR:
                {
                    Bullet newbul;

                    newbul.x          = x+(cos(-angle) * 8.0f);
                    newbul.y          = y+(sin(angle) * 10.0f);
                    newbul.parentID   = id;
                    newbul.angle      = angle;
                    ammo--;

                    bulcon->add(newbul);
                } break;
            case WEAPONTYPE_MINES:
                {
                    Bullet newbul;

                    newbul.x          = x + (cos(-angle) * 8.0f);
                    newbul.y          = y + (sin(angle) * 10.0f);
                    newbul.parentID   = id;
                    newbul.angle      = angle;
                    newbul.frame      = BF_MINE;
                    newbul.type       = WEAPONTYPE_MINES;
                    ammo--;

                    bulcon->add(newbul);

                } break;
            case WEAPONTYPE_SHRINKER:
                {
                    Bullet newbul;

                    newbul.x          = x + (cos(-angle) * 8.0f);
                    newbul.y          = y + (sin(angle) * 10.0f);
                    newbul.parentID   = id;
                    newbul.angle      = angle;
                    newbul.frame      = BF_DUKE_SHRINKER;
                    newbul.type       = WEAPONTYPE_SHRINKER;
                    ammo--;

                    bulcon->add(newbul);
                } break;
            case WEAPONTYPE_SPREAD:
                {
                    for (int i = -2; i < 3; ++i)
                    {
                        Bullet newbul;
                        newbul.parentID = id;
                        newbul.x          = x + (cos(-angle) * 8.0f);
                        newbul.y          = y + (sin(angle) * 10.0f);
                        newbul.angle      = angle + ((0.8f / 5) * i);
                        bulcon->add(newbul);
                    }

                    ammo--;
                }
        }


    }

    return true;
}
//-------------------------------
int Dude::hitIt(Dude& enemy, float vectorx, float vectory, int damage)
{

    if ((CirclesColide(x+vectorx, y-vectory, 4.0f, enemy.x, enemy.y, 8.0f))
            &&(enemy.id!=id))
    {
        enemy.hp -= ((shrinked) ? damage * 0.3f : damage);
        enemy.hit = true;
        enemy.lastDamagedBy = id;
        return enemy.id;
    }

    return -1;
}

//-------------------------------
void Dude::disintegrationAnimation()
{
    stim++;
    frame = skinCount * 4;
    if (stim > 30)
    {
        frame = skinCount * 4 + 1;

        if (stim > 50)
        {
            hp = ENTITY_INITIAL_HP;
            shot = false;
            shrinked = false;
            stim = 0;
            spawn = true; 
        }
    }
}
//------------------------------
void Dude::appearInRandomPlace(bool** map, int mapwidth, int mapheight){
    
    startX = rand() % mapwidth;
    startY = rand() % mapheight;

    while (map[startY][startX])
    {
        startX = rand()%mapwidth;
        startY = rand()%mapheight;
    }

    x = startX * 32.0f;
    y = startY * 32.0f;

}
//----------------------------------------
void Dude::damageAnim()
{
    g = b = 0.f;
    r = 1.0f;
    hittim++;

    if (hittim >= 10)
    {
        hittim = 0;
        hit = false;
        r = g = b = 1.f;
    }
}
//------------------------------
void Dude::reload(int time)
{
    reloadtime++;
    if (reloadtime > time / 2)
    {
        frame = (activeSkin[currentWeapon] + 1) * 4 - 2;
    }

    if (reloadtime == time)
    {
        canAtack = true;
        reloadtime = 0;
    }

}
//-------------------------------
void Dude::chageNextWeapon(){
    currentWeapon++;
    if (currentWeapon >= weaponCount)
    {
        currentWeapon = 0;
    }

    frame = (activeSkin[currentWeapon] + 1) * 4 - 2;

}

//------------------------
void Dude::setupToxicParticles()
{
    ps.setParticleLifetime(100);
    ps.setColors(COLOR(0, 1, 0, 0.3), COLOR(0, 1, 0, 0.3));
    ps.setSystemLifetime(-1);
    ps.setDirIntervals(Vector3D(0.5, 0 , 0), 90);
    ps.setSizes(2, 1);
    ps.setPos(0, 0, 0);
    ps.start();
}
//---------------------------------
void Dude::killShrinked(DArray<Dude>& dudes, unsigned yourIndex)
{

    if (shrinked)
    {
        return;
    }

    for (unsigned i = 0; i < dudes.count(); ++i)
    {
        if (i != yourIndex && !dudes[i].shot && !dudes[i].spawn && dudes[i].shrinked)
        {

            if (CirclesColide(x, y, 10, dudes[i].x, dudes[i].y, 3))
            {
                dudes[i].damage(100);
            }
        }
    }
}
//------------------------------
bool Dude::shrink()
{
    if (!shrinked)
    {
        shrinked = true;
        shrinkTimer = MAX_SHRINKED_TIMER;
        return true;
    }

    return false;

}
