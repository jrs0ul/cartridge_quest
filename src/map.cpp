#include "map.h"

#include <cstdio>
#include <wchar.h>
#include <cassert>
#include "Xml.h"
#include "MapGenerator.h"
#include "Item.h"




Dude* CMap::getPlayer(unsigned clientIdx)
{
    assert(enemyCount + clientIdx < mons.count());
    return &mons[enemyCount + clientIdx];
}
//-----------------------------------------
void CMap::addMonster(Dude &newmonster){
    mons.add(newmonster);
}
//-------------------------------------------
void CMap::removeMonster(int index){
    mons.remove(index);
}
//-------------------------------
void CMap::move(Vector3D v, float size)
{
    mapPos = mapPos + Vector3D(v.x * size, v.y * size , v.z * size);
}


//-----------------------------------------
void CMap::arrangeItemsInPremadeMap()
{
    int ix;
    int iy;

    //let's place the games
    for (int a = 0; a < misionItems; ++a)
    {
        ix = rand() % _width;
        iy = rand() % _height;

        bool found = false;

        for (unsigned long i = 0; i < items.count();i++)
        {
            if (( (float)ix == items[i].x)&&((float)iy == items[i].y))
            {
                found=true;
                break;
            }
        }

        while ((_colide[iy][ix]) || (found) || (tiles[iy][ix] == 35)||(tiles[iy][ix] == 36))
        {
            ix=rand() % _width;
            iy=rand() % _height;
            found=false;

            for (unsigned long i=0;i<items.count();i++)
            {
                if ((ix*1.0f == items[i].x) && (iy*1.0f == items[i].y))
                {
                    found=true;
                    break;
                }
            }
        }

        Items game = pickRandomGameCartridge();
        addItem(ix * TILE_WIDTH, iy * TILE_WIDTH, game);

    }

    placeAmmoAndMedkits();

}


//----------------------------------
Items CMap::pickRandomGameCartridge()
{
    int dice = rand() % 1000;

    Items game = ITEM_GAME_DUKE_ATOMIC;

    if (dice < 800)
    {
        game = ITEM_GAME_CONTRABANDISTS;

        if (dice < 600)
        {
            game = ITEM_GAME_FART_NIGHT;

            if (dice < 400)
            {
                game = ITEM_GAME_UNABOMBER_GUY;

                if (dice < 200)
                {
                    game = ITEM_GAME_SPEEDBALL;
                }
            }
        }

    }

    return game;
}


//-------------------------------------
void CMap::generate(int level)
{

    if (!tiles)
    {
        _width = 20 + level * 10 + (5 - rand() % 10);
        _height = 20 + level * 10 + (5 - rand() % 10);

        tiles = new unsigned char* [_height];

        for (unsigned i = 0; i < _height; ++i)
        {
            tiles[i] = new unsigned char[_width];

            for (unsigned a = 0; a < _width; ++a)
            {
                int other = rand() % 3;
                int grass = rand() % 10;

                tiles[i][a] = (other == 2) ? ((grass == 2) ? 10 : TILE_ROCKS): TILE_DIRT;
            }
        }
    }

    putSlimeCircle();

    MapGenerator gen(_width, _height);
    gen.generate(this);
    buildCollisionmap();

    const unsigned ROOM_COUNT = gen.getRoomCount();

    unsigned exitRoomIdx = rand() % ROOM_COUNT;
    BSPTreeNode* exitRoom = gen.getRoomNode(exitRoomIdx);

    exit.x = exitRoom->startx + exitRoom->roomPosX + exitRoom->roomWidth / 2;
    exit.y = exitRoom->starty + exitRoom->roomPosY + exitRoom->roomHeight / 2;

    tiles[(int)exit.y][(int)exit.x] = TILE_EXIT;

    unsigned startRoomIdx = rand() % ROOM_COUNT;

    while (startRoomIdx == exitRoomIdx)
    {
        startRoomIdx = rand() % ROOM_COUNT;
    }

    BSPTreeNode* startRoom = gen.getRoomNode(startRoomIdx);

    start.x = startRoom->startx + startRoom->roomPosX + startRoom->roomWidth / 2;
    start.y = startRoom->starty + startRoom->roomPosY + startRoom->roomHeight / 2;

    start.x *= TILE_WIDTH;
    start.y *= TILE_WIDTH;


    enemyCount = 0;

    for (unsigned i = 0; i < ROOM_COUNT; ++i)
    {
        if (startRoomIdx == i)
        {
            continue;
        }

        BSPTreeNode* room = gen.getRoomNode(i);

        const int monstersInTheRoom = (room->roomWidth * room->roomHeight) / 20;

        for (int a = 0; a < monstersInTheRoom; ++a)
        {
            Dude m;
            m.id = enemyCount;
            m.race = rand() % MONSTER_MAX_RACE + 1;
            m.initMonsterHP();
            m.x = room->startx + room->roomPosX + (1 + rand() % (room->roomWidth - 3));
            m.y = room->starty + room->roomPosY + (1 + rand() % (room->roomHeight - 3));
            m.x *= TILE_WIDTH;
            m.y *= TILE_WIDTH;
            mons.add(m);

            while (mons[enemyCount - 1].isColideWithOthers(mons, 
                                                           mons[enemyCount - 1].x, 
                                                           mons[enemyCount - 1].y, 
                                                           false,
                                                           enemyCount))
            {
                mons[enemyCount - 1].x = room->startx + room->roomPosX + (1 + rand() % (room->roomWidth - 3));
                mons[enemyCount - 1].y = room->starty + room->roomPosY + (1 + rand() % (room->roomHeight - 3));
                mons[enemyCount - 1].x *= TILE_WIDTH;
                mons[enemyCount - 1].y *= TILE_WIDTH;
            }

            ++enemyCount;
        }
    }

    misionItems = rand() % 2 + 2;
    goods = rand() % 8 + 2;

    for (int i = 0; i < misionItems; ++i)
    {
        int roomIdx = rand() % ROOM_COUNT;
        BSPTreeNode* room = gen.getRoomNode(roomIdx);
        Items game = pickRandomGameCartridge();
        int ix = room->startx + room->roomPosX + (1 + rand() % (room->roomWidth - 2));
        int iy = room->starty + room->roomPosY + (1 + rand() % (room->roomHeight - 2));
        addItem(ix * TILE_WIDTH, iy * TILE_WIDTH, game);
    }

    placeAmmoAndMedkits();

}

//-------------------------------------
bool CMap::load(const char* path, bool createItems, int otherplayers){

    char buf[255];

    puts(path);

    strcpy(name, path);

    printf("Loading [%s]\n", path);

    enemyCount = 0;

    Xml mapfile;

    bool res = mapfile.load(path);

    if (res)
    {
        XmlNode* mainnode = mapfile.root.getNode(L"Map");


        if (mainnode)
        {
            for (unsigned i = 0; i < mainnode->childrenCount(); ++i)
            {
                XmlNode* node = mainnode->getNode(i);

                if (node)
                {
                    if (wcscmp(node->getName(), L"size") == 0)  //  parse size
                    {
                        for (unsigned a = 0; a < node->attributeCount(); ++a)
                        {
                            XmlAttribute *attr = node->getAttribute(a);

                            if (attr)
                            {
                                if (wcscmp(attr->getName(), L"width") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        _width = atoi(buf);
                                        printf("MAP WIDTH: %d\n", _width);
                                    }
                                }
                                else if (wcscmp(attr->getName(), L"height") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        _height = atoi(buf);
                                        tiles  = new unsigned char*[_height];
                                        _colide = new bool* [_height];
                                        printf("MAP HEIGHT: %d\n", _height);
                                    }

                                }
                            }
                        }
                    }
                    else if (wcscmp(node->getName(), L"rows") == 0)  //  parse rows
                    {
                        for (unsigned a = 0; a < node->childrenCount(); ++a)
                        {
                            XmlNode* row = node->getNode(a);

                            if (row)
                            {

                                tiles[a] = new unsigned char[_width];
                                _colide[a] = new bool[_width];

                                for (unsigned j = 0; j < row->childrenCount(); ++j)
                                {
                                    XmlNode* tile = row->getNode(j);

                                    if (tile)
                                    {
                                        sprintf(buf, "%ls", tile->getValue());
                                        tiles[a][j] = atoi(buf) - 48;

                                        _colide[a][j] = (ColidingTiles[tiles[a][j]]) ? true : false;

                                    }

                                }
                            }
                        }
                    }
                    else if (wcscmp(node->getName(), L"start") == 0) // start pos
                    {
                        for (unsigned a = 0; a < node->attributeCount(); ++a)
                        {
                            XmlAttribute *attr = node->getAttribute(a);

                            if (attr)
                            {
                                if (wcscmp(attr->getName(), L"x") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        start.x = atoi(buf);
                                        printf("PLAYER START.X: %f\n", start.x);
                                    }
                                }
                                else if (wcscmp(attr->getName(), L"y") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        start.y = atoi(buf);
                                        printf("PLAYER START.Y: %f\n", start.y);
                                    }

                                }
                            }
                        }
                    }
                    else if (wcscmp(node->getName(), L"exit") == 0) // exit pos
                    {
                        for (unsigned a = 0; a < node->attributeCount(); ++a)
                        {
                            XmlAttribute *attr = node->getAttribute(a);

                            if (attr)
                            {
                                if (wcscmp(attr->getName(), L"x") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        exit.x = atoi(buf);
                                        printf("PLAYER EXIT.X: %f\n", exit.x);
                                    }
                                }
                                else if (wcscmp(attr->getName(), L"y") == 0)
                                {
                                    wchar_t* val = attr->getValue();

                                    if (val)
                                    {
                                        sprintf(buf, "%ls", val);
                                        exit.y = atoi(buf);
                                        printf("PLAYER EXIT.Y: %f\n", exit.y);
                                    }

                                }
                            }
                        }
                    }
                    else if (wcscmp(node->getName(), L"items") == 0) // key item count
                    {
                        sprintf(buf, "%ls", node->getValue());
                        misionItems = atoi(buf);
                    }
                    else if (wcscmp(node->getName(), L"time") == 0) // time to complete
                    {
                        sprintf(buf, "%ls", node->getValue());
                        timeToComplete = atoi(buf);
                    }
                    else if (wcscmp(node->getName(), L"goods") == 0) // goodies
                    {
                        sprintf(buf, "%ls", node->getValue());
                        goods = atoi(buf);
                    }
                    else if (wcscmp(node->getName(), L"enemies") == 0) // goodies
                    {
                        enemyCount = node->childrenCount();

                        for (unsigned a = 0; a < node->childrenCount(); ++a)
                        {
                            XmlNode* enemy = node->getNode(a);

                            if (enemy)
                            {

                                int sx = 0;
                                int sy = 0;
                                Dude naujas;
                                naujas.id = a;

                                for (unsigned a = 0; a < enemy->attributeCount(); ++a)
                                {
                                    XmlAttribute *attr = enemy->getAttribute(a);

                                    if (attr)
                                    {
                                        if (wcscmp(attr->getName(), L"x") == 0)
                                        {
                                            wchar_t* val = attr->getValue();

                                            if (val)
                                            {
                                                sprintf(buf, "%ls", val);
                                                sx = atoi(buf);
                                            }
                                        }
                                        else if (wcscmp(attr->getName(), L"y") == 0)
                                        {
                                            wchar_t* val = attr->getValue();

                                            if (val)
                                            {
                                                sprintf(buf, "%ls", val);
                                                sy = atoi(buf);
                                            }

                                        }
                                    }
                                } // for

                                naujas.startX = sx*32;
                                naujas.startY = sy*32;
                                naujas.x=(float)naujas.startX;
                                naujas.y=(float)naujas.startY;
                                naujas.race=rand() % MONSTER_MAX_RACE + 1;
                                naujas.initMonsterHP();
                                mons.add(naujas);

                            }

                        }
                    }

                }
            }
        }
    }


    mapfile.destroy();

    printf("map width %d, height %d\n", _width, _height);

    start.x = start.x * TILE_WIDTH;
    start.y = start.y * TILE_WIDTH;

    if ((enemyCount + 1 + otherplayers) != 0)
    {

        Dude playeris;
        playeris.id = enemyCount;
        playeris.race = 4;
        playeris.setWeaponCount(PLAYER_SIMULTANEOUS_WEAPONS);
        playeris.setSkinCount(PLAYER_MAX_SKIN_COUNT);
        playeris.setFrame((playeris.activeSkin[playeris.getCurrentWeapon()] + 1) * 4 - 2);
        mons.add(playeris);

        for (int i = 0; i < otherplayers; ++i)
        {
            playeris.id++;
            playeris.appearInRandomPlace(_colide, width(), height());
            mons.add(playeris);
        }
    }



    if (createItems)
    {
        arrangeItemsInPremadeMap();
    }


    return true;
}
//------------------------------------
void CMap::buildCollisionmap()
{

    if (!_colide)
    {
        _colide = new bool * [_height];

        for (unsigned i = 0; i < _height; ++i)
        {
            _colide[i] = new bool[_width];
        }

    }

    for (unsigned i = 0; i < _height; ++i)
    {
        for (unsigned a = 0; a < _width; ++a)
        {
            _colide[i][a] = (ColidingTiles[tiles[i][a]]) ? true : false;
        }
    }
}

//--------------------------------------
bool CMap::save(const char* path)
{
    printf("SAVING %s...\n", path);

    Xml mapfile;

    XmlNode mapnode;
    mapnode.setName(L"Map");
    XmlNode size;
    size.setName(L"size");
    wchar_t width[10];
    swprintf(width, 10, L"%d", _width);
    size.addAtribute(L"width", width);
    wchar_t height[10];
    swprintf(height, 10, L"%d", _height);
    size.addAtribute(L"height", height);
    mapnode.addChild(size);

    XmlNode rows;
    rows.setName(L"rows");

    for (unsigned i = 0; i < _height; ++i)
    {
        XmlNode row;
        row.setName(L"row");

        for (unsigned a = 0; a < _width; ++a)
        {
            XmlNode tile;
            tile.setName(L"t");
            wchar_t value[10];
            swprintf(value, 10, L"%d", tiles[i][a] + 48);
            tile.setValue(value);
            row.addChild(tile);
        }

        rows.addChild(row);
    }
    mapnode.addChild(rows);


    XmlNode startTag;
    startTag.setName(L"start");
    wchar_t buffer[50];
    printf("%d %d\n", (int)start.x/32, (int)start.y/32);
    swprintf(buffer, 50, L"%d", (int)start.x / 32);
    startTag.addAtribute(L"x", buffer);

    swprintf(buffer, 50, L"%d", (int)start.y / 32);
    startTag.addAtribute(L"y", buffer);
    mapnode.addChild(startTag);

    XmlNode exitTag;
    exitTag.setName(L"exit");
    swprintf(buffer, 50, L"%d", (int)exit.x);
    exitTag.addAtribute(L"x", buffer);
    swprintf(buffer, 50, L"%d", (int)exit.y);
    exitTag.addAtribute(L"y", buffer);
    mapnode.addChild(exitTag);

    XmlNode questItemTag;
    questItemTag.setName(L"items");
    swprintf(buffer, 50, L"%d", misionItems);
    questItemTag.setValue(buffer);
    mapnode.addChild(questItemTag);

    XmlNode timeTag;
    timeTag.setName(L"time");
    swprintf(buffer, 50, L"%d", timeToComplete);
    timeTag.setValue(buffer);
    mapnode.addChild(timeTag);

    XmlNode itemsTag;
    itemsTag.setName(L"goods");
    swprintf(buffer, 50, L"%d", goods);
    itemsTag.setValue(buffer);
    mapnode.addChild(itemsTag);

    XmlNode enemies;
    enemies.setName(L"enemies");

    for (unsigned i = 0; i < (unsigned)enemyCount; ++i)
    {
        XmlNode enemyPos;
        enemyPos.setName(L"pos");
        wchar_t buf[10];
        swprintf(buf, 10, L"%d", (int)mons[i].x / 32);
        enemyPos.addAtribute(L"x", buf);
        swprintf(buf, 10, L"%d", (int)mons[i].y / 32);
        enemyPos.addAtribute(L"y", buf);
        enemies.addChild(enemyPos);
    }

    mapnode.addChild(enemies);

    mapfile.root.addChild(mapnode);
    mapfile.write(path);

    mapfile.destroy();

    return true;
}

//--------------------------------------
void CMap::draw(SpriteBatcher& pics, float r, float g, float b, int ScreenWidth, int ScreenHeight)
{

    unsigned tileset = pics.findByName("pics/tileset.tga");

    if (tiles)
    {

        for (unsigned a = 0; a < _height; ++a)
        {

            for (unsigned i = 0; i < _width; ++i)
            {
                const float tileX = i * TILE_WIDTH + mapPos.x;
                const float tileY = a * TILE_WIDTH + mapPos.y;
                if (tileX + HALF_TILE_WIDTH < 0 || tileX - HALF_TILE_WIDTH > ScreenWidth ||
                        tileY + HALF_TILE_WIDTH < 0 || tileY - HALF_TILE_WIDTH > ScreenHeight)
                {
                    continue;
                }

                pics.draw(tileset,
                        tileX,
                        tileY,
                        tiles[a][i] - 1,
                        true,
                        1.f, 1.f, 0.f, COLOR(r, g, b, 1.0f), COLOR(r, g, b, 1.0f));

            }

        }
    }


    for (unsigned i = 0; i < decals.count(); i++)
    {
        decals[i].draw(pics, 15, mapPos.x, mapPos.y, ScreenWidth, ScreenHeight);
    }


    for (unsigned long i = 0; i < items.count(); ++i)
    {
        const int itemPicture = (items[i].value > ITEM_MEDKIT) ? 11 : 7;
        const int itemFrame = (items[i].value > ITEM_MEDKIT) ? items[i].value - ITEM_GAME_NINJA_MAN :
                                                               items[i].value - 1;

        const float itemX = items[i].x + mapPos.x;
        const float itemY = items[i].y + mapPos.y;

         if (itemX + HALF_TILE_WIDTH < 0 || itemX - HALF_TILE_WIDTH > ScreenWidth ||
                        itemY + HALF_TILE_WIDTH < 0 || itemY - HALF_TILE_WIDTH > ScreenHeight)
                {
                    continue;
                }


        pics.draw(itemPicture,
                  itemX,
                  itemY,
                  itemFrame,
                  true,
                  1.0f,
                  1.0f,
                  0.0,
                  COLOR(r,g,b, 1.0f),
                  COLOR(r,g,b, 1.0f)
                );
    }

}
//--------------------------------------
void CMap::drawEntities(SpriteBatcher& pics, int ScreenWidth, int ScreenHeight)
{

    for (unsigned i = 0; i < mons.count(); i++)
    {
        if (mons[i].shrinked)
        {
            mons[i].draw(pics, mons[i].race + 1, mapPos.x, mapPos.y, ScreenWidth, ScreenHeight);
        }
    }


    for (unsigned i = 0; i < mons.count(); i++)
    {
        if (!mons[i].shrinked)
        {
            mons[i].draw(pics, mons[i].race + 1, mapPos.x, mapPos.y, ScreenWidth, ScreenHeight);
        }
    }
}

//--------------------------------------
void CMap::destroy()
{

    mons.destroy();

    if (tiles)
    {
        for (unsigned a=0; a < _height; a++)
        {
            delete []tiles[a];
        }

        delete []tiles;
        tiles=0;
    }

    if (_colide)
    {
        for (unsigned a=0; a < _height; a++)
        {
            delete []_colide[a];
        }

        delete []_colide;
        _colide=0;
    }

    items.destroy();
    decals.destroy();


}
//-------------------------------------------
bool CMap::colide(unsigned x, unsigned y)
{

    if (!_colide)
    {
        return false;
    }

    return _colide[y][x];

}

//-------------------------------------------


void CMap::ReplaceTiles(unsigned char old, unsigned char fresh){
 for (unsigned a = 0; a < _height; a++)
  for (unsigned i=0;i < _width;i++)
  {
    if (tiles[a][i]==old)
    {
        tiles[a][i]=fresh;
    }
  }
}

//-------------------------------------------
void CMap::addItem(float nx, float ny, int nvalue)
{
    Item newitem;
    newitem.x = nx;
    newitem.y = ny;
    newitem.value = nvalue;
    printf("ADDING ITEM %d\n", nvalue);
    items.add(newitem);

}

//-------------------------------------------
void CMap::removeItem(int ID){
    items.remove(ID);

}

//-------------------------------------
void CMap::fadeDecals()
{
    for (unsigned int i = 0; i < decals.count(); i++)
    {
        decals[i].color.a -= 0.0005f;
        if (decals[i].color.a <= 0.0f)
        {
            decals.remove(i);
        }
    }

}
//------------------------

int CMap::findCreatureById(int id)
{
    for (unsigned i = 0; i < mons.count(); ++i)
    {
        if (mons[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

//----------------------
void CMap::placeAmmoAndMedkits()
{
    int ix = 0;
    int iy = 0;

    for (int a = 0; a < goods; ++a)
    {
        ix = rand() % _width;
        iy = rand() % _height;

        bool found = false;

        for (unsigned long i = 0; i < items.count(); ++i)
        {
            if ((ix == (int)items[i].x) && (iy == (int)items[i].y))
            {
                found = true;
                break;
            }
        }

        while ((_colide[iy][ix]) || (found))
        {
            ix=rand() % _width;
            iy=rand() % _height;

            found = false;

            for (unsigned long i=0;i<items.count();i++)
            {
                if ((ix*1.0f==items[i].x)&&(iy*1.0f==items[i].y))
                {
                    found=true;
                    break;
                }
            }
        }

        const Items theItem = ((rand() % 2 + 1) == 2) ? ITEM_MEDKIT : ITEM_AMMO_PACK;
        addItem(ix * TILE_WIDTH, iy * TILE_WIDTH, theItem);

    }

}
//--------------------------------
void CMap::putSlimeCircle()
{
    float circleX = 0;
    float circleY = 0;

    float posX = rand() % (_width * TILE_WIDTH);
    float posY = rand() % (_height * TILE_WIDTH);

    for (int a = 0; a < 150; a += 10)
    {
        for (float i = 0; i < 2 * M_PI; i += 0.1f)
        {
            circleY  = posY + (sinf(i) * a);
            circleX  = posX + (cosf(i) * a);

            unsigned x = (unsigned)roundf(circleX / TILE_WIDTH);
            unsigned y = (unsigned)roundf(circleY / TILE_WIDTH);

            if (x < _width && y < _height)
            {
                tiles[y][x] = TILE_SLIME + rand() % 2;
            }
        }
    }
}
