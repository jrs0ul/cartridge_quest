#ifndef DUDE_H
#define DUDE_H

#include "DArray.h"
#include "Vectors.h"
#include "Consts.h"
#include "Particles2D.h"
#include "WeaponTypes.h"

class CBulletContainer;
class SpriteBatcher;
class CMap;


enum MonsterRaces
{
    MONSTER_RACE_BULL = 1,
    MONSTER_RACE_GOBBLER,
    MONSTER_RACE_COP,
    MONSTER_RACE_PLAYER
};

class Dude
{
        Vector3D oldPos[ENTITY_POSITION_HISTORY_LEN];
        Particle2DSystem ps;
        int hp;
        int weaponCount;
        int skinCount;
        int currentWeapon;
        int shrinkTimer;
        unsigned char frame;
public:
        float x;
        float y;
        float angle;
        float r;
        float g;
        float b;

        int item;
        int atim;
        int delay;

        int victimID;

        int id;
        int tim;
        int stim;

        int race;

        int ammo;
        int reloadtime;
        int startX;
        int startY;

        int hittim;

        int equipedGame;


        int lastDamagedBy;

        unsigned char activeSkin[2];  //what frames to use for each weapon

        unsigned char colid;
        bool enemyseen;
        bool hit;
        bool canAtack;
        bool spawn;
        bool shot;
        bool shrinked;



        Dude(float dx = 0, float dy = 0)
        : hp(ENTITY_INITIAL_HP)
        , weaponCount(1)
        , skinCount(1)
        , currentWeapon(0)
        , shrinkTimer(0)
        , frame(0)
        , x(dx)
        , y(dy)
        , angle(0.f)
        , shot(false)
        , shrinked(false)
        {
            tim = 0;
            stim = 0;
            ammo = ENTITY_INITIAL_AMMO;
            canAtack = true;
            reloadtime = 0;
            r = g = b = 1.0f;
            hit = false;
            hittim = 0;
            id = 0;

            victimID = 0;
            colid = 0;
            atim = 0;
            enemyseen = false;
            item = 0;
            delay = rand()%2+1;

            equipedGame = 0;

            spawn=false;
            activeSkin[0] = 0;
            activeSkin[1] = 1;

            for (int i = 0; i < ENTITY_POSITION_HISTORY_LEN; ++i)
            {
                oldPos[i] = Vector3D(dx, dy, 0);
            }
        }

        void update();

        void rotate(float angle);
        bool move(float walkSpeed, float strifeSpeed, float radius, CMap& map, bool isCoop);
        bool moveGamePad(const Vector3D& movementDir,float radius, CMap& map, bool isCoop);
        void respawn();

        void draw(SpriteBatcher& pics, unsigned index, float posx, float posy, int ScreenWidth, int ScreenHeight);
        void drawParticles(SpriteBatcher& pics, float posx, float posy, int ScreenWidth, int ScreenHeight);

        /* shoots or deploys a mine if the entity has some ammo */
        bool shoot(bool useBullets, WeaponTypes weaponType, CBulletContainer* bulcon);
        //melee attack
        int hitIt(Dude& enemy, float vectorx, float vetory, int damage);

        void heal();

        void disintegrationAnimation();
        void appearInRandomPlace(bool** map, int mapwidth, int mapheight);
        //gets red if damaged
        void damageAnim();
        // waits so it could attack again
        void reload(int time);
        void chageNextWeapon();

        int getHP(){return hp;}
        void initMonsterHP();
        void setHP(int newHP){hp = newHP;}
        //true if hp <= 0
        bool damage(int dmg);

        void setWeaponCount(int newCount){weaponCount = newCount; currentWeapon = 1;}
        void setupToxicParticles();
        void setSkinCount(int cnt){skinCount = cnt;}
        void setFrame(unsigned char newFrame){frame = newFrame;}
        unsigned char getFrame(){return frame;}
        int getCurrentWeapon(){return currentWeapon;}
        void killShrinked(DArray<Dude>& dudes, unsigned yourIndex);
        void stopParticles(){ps.stop();}
        bool shrink();
        bool isColideWithOthers(DArray<Dude>& chars, float newx, float newy, bool coop, unsigned monsterCount);

private:
        bool movement(Vector3D dir,
                      float radius,
                      const bool** map,
                      int mapsizex,
                      int mapsizey,
                      DArray<Dude>& chars,
                      bool coop,
                      unsigned monsterCount);

};


//-------



#endif //DUDE_H
