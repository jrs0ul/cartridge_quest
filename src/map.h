#ifndef MAP_H
#define MAP_H


#include "DArray.h"
#include "Dude.h"
#include "Decal.h"
#include "Item.h"



struct Item;
class MapGenerator;


const int ColidingTiles[256] = {
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //9
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //19
                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //29
                                1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //39
                                0, 0, 0, 0, 1, 1, 1, 1, 0, 0, //49
                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //59
                                1, 1, 1, 1, 1, 1, 0, 1, 0, 1, //69
                                0, 1, 0, 0, 0, 0, 0, 0, 0, 0, //79
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                                };

class CMap 
{

        Vector3D mapPos; //map starting position on the screen
public:
        DArray<Item> items;
        DArray<Dude> mons;
        DArray<Decal> decals;
        unsigned char** tiles;
        bool** _colide;
        char name[20];
        Vector3D start;
        Vector3D exit;
        unsigned _width;
        unsigned _height;
        int itmframe;

        int timeToComplete;
        int misionItems;
        int goods; // ammo, medkits, etc.
        int enemyCount;



        CMap() : 
        _width(0),
        _height(0),
        itmframe(0)
        {
            tiles = 0;
            timeToComplete = 0;
            misionItems = 0;
            goods = 0;
            enemyCount = 0;
        }

        Dude* getPlayer(unsigned clientIdx = 0);

        unsigned width(){return _width;}
        unsigned height(){return _height;}
        void setWidth(unsigned newWidth){_width = newWidth;}
        void setHeight(unsigned newHeight){_height = newHeight;}

        Vector3D getPos(){return mapPos;}
        void setPosX(float value){mapPos.x = value;}
        void setPosY(float value){mapPos.y = value;}
        void move(Vector3D v, float size);

        void generate(int level);
        bool load(const char* path, bool createItems=true, int otherplayers=0);
        bool save(const char* path);

        void buildCollisionmap();

        void draw(SpriteBatcher& pics, float r, float g, float b, int ScreenWidth, int ScreenHeight);

        void drawEntities(SpriteBatcher& pics, int ScreenWidth, int ScreenHeight);

        bool colide(unsigned x, unsigned y);
        void ReplaceTiles(unsigned char old, unsigned char fresh);
        void addItem(float nx, float ny, int nvalue);
        void removeItem(int ID);
        void addMonster(Dude& newmonster);
        void removeMonster(int index);
        void fadeDecals();
        int findCreatureById(int id);

        void destroy();

    private:
        void arrangeItemsInPremadeMap();
        Items pickRandomGameCartridge();
        void placeAmmoAndMedkits();
        void putSlimeCircle();

};




#endif //MAP_H
