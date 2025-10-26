#ifndef CBULL_CON_H
#define CBULL_CON_H

#include "DArray.h"


class Bullet;
class SpriteBatcher;
class Dude;

class CBulletContainer
{
public:
    DArray<Bullet> buls;


    CBulletContainer(){}
    int count(){return buls.count();};
    void add(Bullet& newbulet);
    void draw(SpriteBatcher& pics, float posx, float posy, int ScreenWidth, int ScreenHeight);
    void update(const bool** colisionGrid, DArray<Dude>& dudes, int mapWidth, int mapHeight);
    void destroy();
    ~CBulletContainer(){}
private:
    void removeDead();


};

 

#endif // CBULL_CON_H

