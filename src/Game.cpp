#include "Game.h"

#include <cstdio>
#include <ctime>
#include <cassert>

#include "Dude.h"
#include "BulletContainer.h"
#include "SpriteBatcher.h"
#include "bullet.h"
#include "gui/Slider.h"
#include "gui/Text.h"
#include "Usefull.h"
#include "SpriteBatcher.h"
#include "Matrix.h"
#include "audio/OggStream.h"
#include "Intro.h"
#include "Item.h"
#include "Consts.h"
#include "SaveGame.h"
#ifndef _WIN32
#include <arpa/inet.h>
#endif


Game::Game()
: imgCount(0)
{
    Accumulator = 0;
    DT = 1000.0f / 60.0f / 1000.0f;
    Works = true;
    TimeTicks = 0;

    DebugMode = 0;

    state = GAMESTATE_TITLE;
    netGameState = MPMODE_COOP;


    otherClientCount = 0;
    maxwavs=0;

    noAmmo = false;

    frags = 0;
    timeleft=0;
    showdebugtext=false;
    FirstTime = true;
    gameOver = false;
    ms=0;

    mapas.itmframe = 0;
    itmtim = 0;
    door_tim = 0;
    fadeTimer = 0;
    netMode = NETMODE_NONE;
    clientInfoSendCounter = 0;
    Client_GotMapData = false;
    clientMyIndex = 0;
    slimeTimer = 0;
    objectivetim = 200;
    doRumble = false;
    showMiniMap = false;
    doFadein = true;
    godmode = false;
    slimeswap = false;
}
//-------------------------------------
void PlaySoundAt(SoundSystem* ss, float x, float y, int soundIndex)
{
    ss->setSoundPos(soundIndex, Vector3D(x, 0, y).v);
    ss->playsound(soundIndex);
}
//---------------------------------------
void AdaptSoundPos(int soundIndex, float soundx,float soundy){

    SoundSystem* ss = SoundSystem::getInstance();
    Vector3D v = Vector3D(soundx, 0, soundy);
    ss->setSoundPos(soundIndex, v.v);
}
//-----------------------------------------------------------
int Game::FPS()
{
    static int ctime = 0, FPS = 0, frames = 0, frames0 = 0;
    if ((int)TimeTicks >= ctime) {
        FPS = frames - frames0;
        ctime = (int)TimeTicks + 1000;
        frames0 = frames;
    }
    frames = frames+1;
    return FPS;
}


//-------------------------------------------

void Game::DeleteAudio()
{
    music.stop();
    music.release();
}


//----------------------------------
bool Game::InitAudio()
{

    SoundSystem::getInstance()->init(0);
    SoundSystem::getInstance()->loadFiles("sfx/", "list.txt");

    return true;
}

//----------------------------------------
void Game::PlayNewSong(const char* songName)
{
    if (music.playing())
    {
        music.stop();
        music.release();
    }

    char buf[255];
    sprintf(buf,"music/%s",songName);
    music.open(buf);
    //music.setVolume(sys.musicVolume);
    music.playback();
}


//-------------------

void Game::DrawSomeText()
{

    char buf[255];

    sprintf(buf,"FPS : %d DeltaTime %f",FPS(), DeltaTime);
    WriteText(20,20, pics, 10, buf, 0.8f,0.6f);

    sprintf(buf, "Map width %d height %d", mapas.width(), mapas.height());
    WriteText(20, 40, pics, 10, buf, 0.8f,0.8f);

    if (netMode == NETMODE_SERVER)
    {
        sprintf(buf,"Client count : %d",serveris.clientCount());
        WriteText(20, 60, pics, 10,buf, 0.8f, 1);
    }

    for (unsigned i = 0; i < mapas.mons.count(); ++i)
    {
        sprintf(buf, "mons[%d] id=%d x=%.2f hp=%d shot=%d", 
                i, mapas.mons[i].id, mapas.mons[i].x, mapas.mons[i].getHP(), mapas.mons[i].shot);
        WriteText(20, 80 + 20 * i, pics, 10, buf, 0.8f, 0.8f);
    }

    for (unsigned i = 0; i < fragTable.count(); ++i)
    {
        sprintf(buf, "%d. %d frags", i, fragTable[i]);
        WriteText(20, (sys.ScreenHeight - 100) + 20 * i, pics, 10, buf, 0.8f, 0.8f);

    }


}


//------------------
void Game::DrawMap(float r=1.0f,float g=1.0f, float b=1.0f)
{

    mapas.draw(pics, r, g, b, sys.ScreenWidth, sys.ScreenHeight);

    bulbox.draw(pics, mapas.getPos().x, mapas.getPos().y, sys.ScreenWidth, sys.ScreenHeight);

    mapas.drawEntities(pics, sys.ScreenWidth, sys.ScreenHeight);

    for (unsigned i = 0; i < mapas.mons.count(); ++i)
    {
        mapas.mons[i].drawParticles(pics, mapas.getPos().x, mapas.getPos().y, sys.ScreenWidth, sys.ScreenHeight);
    }


}

//-------------------------------------------
void Game::DrawMiniMap(int x, int y)
{

    const int MINIMAP_TILESET = 12;

    pics.draw(MINIMAP_TILESET, x, y, 0, false ,mapas.width(), mapas.height(), 0, COLOR(1,1,1, 0.6f), COLOR(1,1,1, 0.6f));

    for (unsigned i = 0; i < mapas.height(); i++)
    {
        for (unsigned a = 0; a < mapas.width(); a++)
        {
            int frame = 0;

            if ( mapas.tiles[i][a] == TILE_V_DOOR_DIRT || mapas.tiles[i][a] == TILE_V_DOOR_CONCRETE)
            {
                frame = 4;
            }
            else if (mapas.tiles[i][a] == TILE_H_DOOR_DIRT || mapas.tiles[i][a] == TILE_H_DOOR_CONCRETE)
            {
                frame = 5;
            }
            else if (mapas.tiles[i][a] == TILE_EXIT)
            {
                frame = 6;
            }
            else
            {
                frame = mapas.colide(a, i);
            }

            if (frame)
            {
                pics.draw(MINIMAP_TILESET,
                          a * MINIMAP_TILE_WIDTH + x,
                          i * MINIMAP_TILE_WIDTH + y,
                          frame, false, 1.f, 1.f, 0.f, COLOR(1,1,1,0.6f), COLOR(1,1,1,0.6f));
            }
        }
    }

    Dude* player = mapas.getPlayer((netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0);

    pics.draw(MINIMAP_TILESET,
              x + (round(player->x / TILE_WIDTH) * MINIMAP_TILE_WIDTH),
              y + (round(player->y / TILE_WIDTH) * MINIMAP_TILE_WIDTH),
              3,
              false);

    for (unsigned i = 0; i<mapas.items.count(); i++)
    {
        pics.draw(MINIMAP_TILESET,
                  x + (round(mapas.items[i].x / TILE_WIDTH) * MINIMAP_TILE_WIDTH),
                  y + (round(mapas.items[i].y / TILE_WIDTH) * MINIMAP_TILE_WIDTH),
                  2,
                  false);
    }


}

//---------------------
void Game::DrawNumber(int x, int y,int num)
{
    int arr[3] = {0};

    int i = 2;

    while (num != 0)
    {
        arr[i] = num % 10;
        num = num / 10;
        i--;
    }

    for (int a = 0; a < 3; a++)
    {
        pics.draw(8, x + a*16, y, arr[a], false, 0.98f, 1.f, 0.f, COLOR(1.f, 1.f, 1.f, 0.6f), COLOR(1.f, 1.f, 1.f, 0.6f));
    }
}
//-------------------------------
void Game::SendItemCreation(float x, float y, int value, unsigned int clientIndex)
{

    char buferis[MAX_MESSAGE_DATA_SIZE];
    int index = 0;

    strcpy(buferis, NET_HEADER);
    index += NET_HEADER_LEN;
    buferis[index] = NET_SERVER_MSG_NEW_ITEM;
    ++index;
    memcpy(&buferis[index], &x, sizeof(float));
    index += sizeof(float);
    memcpy(&buferis[index], &y, sizeof(float));
    index += sizeof(float);
    memcpy(&buferis[index], &value, sizeof(int));
    index += sizeof(int);
    serveris.sendData(clientIndex, buferis, index);

}

//---------------------------
void Game::KillPlayer(int index)
{
    assert((unsigned)index < mapas.mons.count());
    printf("PLAYER KILLED BY %d\n", mapas.mons[index].lastDamagedBy);

    Dude* player = &mapas.mons[index];

    player->shot = true;
    player->setFrame(PLAYER_MAX_SKIN_COUNT * 4);
    AdaptSoundPos(2, player->x, player->y);
    SoundSystem::getInstance()->playsound(2);

    mapas.mons[index].stim = 0;
    mapas.mons[index].setHP(100);

    Decal decalas;
    decalas.x = player->x;
    decalas.y = player->y;
    decalas.color = COLOR(1.f, 0.f, 0.f);
    decalas.frame = rand() % 2;
    mapas.decals.add(decalas);
}

//------------------
void Game::KillEnemy(unsigned ID)
{

    if (ID < (unsigned)mapas.enemyCount)
    {
        printf("MONSTER KILLED BY %d\n", mapas.mons[ID].lastDamagedBy);
        mapas.mons[ID].shot = true;

        AdaptSoundPos(3,mapas.mons[ID].x,mapas.mons[ID].y);
        SoundSystem::getInstance()->playsound(3);

        Decal decalas;
        decalas.x = round(mapas.mons[ID].x);
        decalas.y = round(mapas.mons[ID].y);
        decalas.color = COLOR(1.f, 0.f, 0.f);
        decalas.frame = rand() % 2;
        mapas.decals.add(decalas);

        if (mapas.mons[ID].item) // if monster swallowed an item
        {
            Item swallowedItem;
            swallowedItem.value = mapas.mons[ID].item;
            swallowedItem.x     = mapas.mons[ID].x;
            swallowedItem.y     = mapas.mons[ID].y;
            mapas.addItem(swallowedItem.x, swallowedItem.y, swallowedItem.value);

            mapas.mons[ID].item = 0;

            if (netMode == NETMODE_SERVER)
            {
                for (unsigned int i = 0; i < serveris.clientCount(); i++)
                {
                    SendItemCreation(swallowedItem.x, swallowedItem.y, swallowedItem.value, i);
                }
            }

        }
    }
    else
    {
        KillPlayer(ID);
    }



}

//---------------------------
void Game::DrawStats()
{
    const unsigned STAT_ICON = 9;
    Dude* player = mapas.getPlayer((netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0);


    pics.draw(STAT_ICON,
              30,
              sys.ScreenHeight - 40,
              2, false, 1.f, 1.f, 0.f, 
              COLOR(1.f, 1.f, 1.f, 0.6f), 
              COLOR(1.f, 1.f, 1.f, 0.6f));
    DrawNumber(58,
            sys.ScreenHeight - 40,
            player->getHP());


    pics.draw(STAT_ICON,
              120,
              sys.ScreenHeight - 40,
              0, false, 1.f, 1.f, 0.f, COLOR(1.f, 1.f, 1.f, 0.6f), COLOR(1.f, 1.f, 1.f, 0.6f)); 
    DrawNumber(155,
            sys.ScreenHeight - 40,
            player->ammo);

    if (netGameState == MPMODE_DEATHMATCH)
    {
        pics.draw(STAT_ICON, 220, sys.ScreenHeight - 40, 3, false, 1.f, 1.f, 0.f, COLOR(1.f, 1.f, 1.f, 0.6f), COLOR(1.f, 1.f, 1.f, 0.6f));
        DrawNumber(255, sys.ScreenHeight - 40, frags);
    }

    if (player->equipedGame)
    {
        pics.draw(11, sys.ScreenWidth / 2 - 32,
                      sys.ScreenHeight - 64, 
                      player->equipedGame - ITEM_GAME_NINJA_MAN, 
                      false, 2.f, 2.f, 0.f, COLOR(1.f, 1.f, 1.f, 0.9f), COLOR(1.f, 1.f, 1.f, 0.9f));
    }


    if (mapas.timeToComplete)
    {
        int min = timeleft / 60;
        int sec = timeleft - min * 60;
        char buf[50];
        sprintf(buf,"%d:%d",min,sec);
        WriteText(450, 20, pics, 10, buf, 0.7f,1.5f, COLOR(1.0f,0.5f,0.5f, 1.f), COLOR(1.0f,0.5f,0.5f, 1.f));
    }

}

//------------------------
//the hero and camera movement
void Game::MoveDude()
{
    Dude* player = mapas.getPlayer((netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0);


    const float SPEED = (player->equipedGame == ITEM_GAME_SPEEDBALL) ? ENTITY_SPEEDBALL_SPEED : 1.f;

    if ((Keys[0]) || (Keys[1]) || (Keys[2]) || (Keys[3]))
    {
        float walkSpeed = 0.f;
        float strifeSpeed = 0.f;

        if (Keys[1])
        {
            walkSpeed = -SPEED;
        }

        if (Keys[0])
        {
            walkSpeed = SPEED;
        }

        if (Keys[2])
        {
            strifeSpeed = -SPEED;
        }

        if (Keys[3])
        {
            strifeSpeed = SPEED;
        }


        player->move(walkSpeed, strifeSpeed, PLAYER_RADIUS, mapas, (netGameState == MPMODE_COOP));

        AdaptMapView();

    }

    

}



//--------------
void Game::AdaptMapView()
{

    int clientIndex = (netMode == NETMODE_CLIENT)? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);

    const float HALF_SCREEN_W = sys.ScreenWidth / 2.f;
    const float HALF_SCREEN_H = sys.ScreenHeight / 2.f;

    float posX = HALF_SCREEN_W - player->x;

    if (posX > HALF_TILE_WIDTH)
    {
        posX = HALF_TILE_WIDTH;
    }
    else if (posX < -1.f * (mapas.width() * TILE_WIDTH - sys.ScreenWidth - HALF_TILE_WIDTH))
    {
        posX = -1.f * (mapas.width() * TILE_WIDTH - sys.ScreenWidth - HALF_TILE_WIDTH);
    }


    float posY = HALF_SCREEN_H - player->y;

    if (posY > HALF_TILE_WIDTH)
    {
        posY = HALF_TILE_WIDTH;
    }
    else if (posY < -1.f * (mapas.height() * TILE_WIDTH - sys.ScreenHeight - HALF_TILE_WIDTH))
    {
        posY = -1.f * (mapas.height() * TILE_WIDTH - sys.ScreenHeight - HALF_TILE_WIDTH);
    }



    if ((int)(mapas.width() * TILE_WIDTH) <= sys.ScreenWidth )
    {
        posX = HALF_SCREEN_W - (mapas.width() * HALF_TILE_WIDTH - HALF_TILE_WIDTH);
    }

    if ((int)(mapas.height() * TILE_WIDTH) <= sys.ScreenHeight)
    {
        posY = HALF_SCREEN_H- (mapas.height() * HALF_TILE_WIDTH - HALF_TILE_WIDTH);
    }


    mapas.setPosX(posX);
    mapas.setPosY(posY);

}


//----------------------------
void Game::GoToLevel(int currentHp, int currentAmmo, int level, int otherplayer)
{

    if (netGameState == MPMODE_DEATHMATCH)
    {
        char mapname[255];
        mapai.getMapName(level, mapname);

        LoadTheMap(mapname, true, otherplayer, currentHp);
    }
    else
    {
        GenerateTheMap(level, currentHp, currentAmmo);
    }

    doFadein = true;
    objectivetim = 200;

    if (showMiniMap)
    {
        showMiniMap = false;
    }

    if (inventory.active())
    {
        inventory.deactivate();
        inventory.reset();
    }
}
//----------------------------------
void Game::SendKillCommandToClient(unsigned clientIdx, int victimId)
{
    char buffer[MAX_MESSAGE_DATA_SIZE];
    int pos = 0;

    memcpy(&buffer[pos], NET_HEADER, NET_HEADER_LEN);
    pos += NET_HEADER_LEN;
    buffer[pos] = NET_SERVER_MSG_KILL_CHARACTER;
    ++pos;

    memcpy(&buffer[pos], &victimId, sizeof(int));
    pos += sizeof(int);
    serveris.sendData(clientIdx, buffer, pos);
}
//----------------------------------
void Game::SendFragsToClient(int clientIdx, int frags)
{
    char buffer[MAX_MESSAGE_DATA_SIZE];
    int len = 0;
    memcpy(&buffer[len], NET_HEADER, NET_HEADER_LEN);
    len += NET_HEADER_LEN;
    buffer[len] = NET_SERVER_MSG_FRAG;
    ++len;
    memcpy(&buffer[len], &frags, sizeof(int));
    len += sizeof(int);

    serveris.sendData(clientIdx, buffer, len);
    printf("sent %d frags to client %d\n", frags, clientIdx);
}
//-----------------------------------
//The client sends information about the taken item to the server
void Game::SendItemCRemove(int itemIndex)
{
    char bufer[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    strcpy(bufer, NET_HEADER);
    index += NET_HEADER_LEN;
    bufer[index] = NET_CLIENT_MSG_ITEM;
    ++index;
    memcpy(&bufer[index], &itemIndex, sizeof(int));
    index += sizeof(int);
    client.sendData(bufer, index);
}
//-----------------------------------
void Game::SendItemSRemove(int ItemIndex, int clientIndex, bool playerTaked)
{
    char bufer[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    memcpy(&bufer[index], NET_HEADER, NET_HEADER_LEN);
    index += NET_HEADER_LEN;
    bufer[index] = NET_SERVER_MSG_ITEM;
    ++index;
    memcpy(&bufer[index],&ItemIndex,sizeof(int));
    index += sizeof(int);
    unsigned char plt = 0;

    if (playerTaked)
    {
        plt = 1;
    }

    memcpy(&bufer[index],&plt,sizeof(unsigned char));
    index++;
    serveris.sendData(clientIndex,bufer,index);
}

//-------------------------------------
void Game::SendMapInfo(int clientIndex, CMap& map)
{
    char bufer[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    memcpy(&bufer[index], NET_HEADER, NET_HEADER_LEN);
    index += NET_HEADER_LEN;
    bufer[index] = NET_SERVER_MSG_SERVER_INFO;

    ++index;
    int len = (int)strlen(map.name); 

    memcpy(&bufer[index], &len, sizeof(int)); //map name len
    index += sizeof(int);

    memcpy(&bufer[index], map.name, strlen(map.name)); //map name
    index += (int)strlen(map.name);

    unsigned int totalClientCount = serveris.clientCount(); 
    memcpy(&bufer[index], &totalClientCount, sizeof(unsigned int)); //client count
    index += sizeof(unsigned int);

    for (unsigned i = 0; i < serveris.clientCount(); ++i)        // all the IDs
    {
        int id = map.mons[map.enemyCount + 1 + i].id;
        memcpy(&bufer[index], &id, sizeof(int));
        index += sizeof(int);
    }

    memcpy(&bufer[index], &clientIndex, sizeof(int)); //your index in the server
    index += sizeof(int);

    char gameType = (char)netGameState;
    bufer[index] = gameType;                        //  the game type : coop or deathmatch
    ++index;

    serveris.sendData(clientIndex, bufer, index);

}
//------------------------------------------------
void Game::SendMapData(int clientIndex, CMap& map)
{

    char bufer[MAX_MESSAGE_DATA_SIZE];

    int index = 0;


    strcpy(bufer, NET_HEADER);
    index += NET_HEADER_LEN;
    bufer[index] = NET_SERVER_MSG_MAP_DATA;
    ++index;

    int height = (netGameState == MPMODE_DEATHMATCH) ? 0 : map.height();
    int width =  (netGameState == MPMODE_DEATHMATCH) ? 0 : map.width();

    memcpy(&bufer[index], &height, sizeof(int));
    index += sizeof(int);
    memcpy(&bufer[index], &width, sizeof(int));
    index += sizeof(int);

    if (width && height)
    {
        unsigned char tile = 0;

        for (unsigned i = 0; i < map.height(); ++i)
        {
            for (unsigned a = 0; a < map.width(); ++a)
            {
                tile = map.tiles[i][a];
                memcpy(&bufer[index], &tile, sizeof(unsigned char));
                index += sizeof(unsigned char);
            }
        }
    }

    assert(index < MAX_MESSAGE_DATA_SIZE);

    int tmp = 0;
    memcpy(&bufer[index], &tmp, sizeof(int));
    index += sizeof(int);

    memcpy(&bufer[index], &map.enemyCount, sizeof(int));
    index += sizeof(int);

    for (int i = 0; i < map.enemyCount; i++)
    {
        memcpy(&bufer[index], &map.mons[i].race, sizeof(int));
        index += sizeof(int);
    }

    int itmcount = (int)map.items.count();
    memcpy(&bufer[index], &itmcount, sizeof(int));
    index+=sizeof(int);

    for (unsigned long i = 0; i < map.items.count(); i++)
    {
        memcpy(&bufer[index], &map.items[i].x,sizeof(float));
        index += sizeof(float);
        memcpy(&bufer[index], &map.items[i].y,sizeof(float));
        index += sizeof(float);
        memcpy(&bufer[index], &map.items[i].value,sizeof(int));
        index += sizeof(int);

    }

    serveris.sendData(clientIndex,bufer,index);
}

//---------------------------------
//Client sends a message to change the map
void Game::SendWarpMessage()
{
    char buferis[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    strcpy(buferis, NET_HEADER);
    index += NET_HEADER_LEN;
    buferis[index] = NET_CLIENT_MSG_NEXT_LEVEL;
    ++index;
    client.sendData(buferis, index);
}


//-------------------------
void Game::ItemPickup()
{
    int clientIndex = (netMode == NETMODE_CLIENT)? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);

    for (unsigned long i = 0; i < mapas.items.count(); ++i)
    {
        if (CirclesColide(player->x, player->y, PLAYER_RADIUS, mapas.items[i].x, mapas.items[i].y, ITEM_RADIUS))
        {

            int item = mapas.items[i].value;

            if ((item != 0) && ((item != ITEM_MEDKIT) || (player->getHP() < ENTITY_INITIAL_HP)))
            {  //  the item will be taken

                SoundSystem* ss = SoundSystem::getInstance();

                if (item == ITEM_MEDKIT)
                {
                    PlaySoundAt(ss, player->x, player->y, 6);
                }
                else
                {
                    PlaySoundAt(ss, player->x, player->y, 1);
                }

                switch (netMode)
                {
                    case NETMODE_NONE : break;
                    case NETMODE_CLIENT : SendItemCRemove(i); break;
                    case NETMODE_SERVER :
                    {
                        for (unsigned int a = 0; a < serveris.clientCount(); a++)
                        {
                            SendItemSRemove(i, a, true);
                        }
                    }
                }
                //------------------------
                mapas.removeItem(i);

                if ((item > ITEM_MEDKIT) && (item > 0))
                {
                    printf("LOOTING ITEM %d\n", item);
                    loot.add(item);
                }
                else
                {
                    if (item == ITEM_AMMO_PACK)
                    {
                        player->ammo += AMMO_PICKUP_VALUE;
                    }
                    else if (item == ITEM_MEDKIT)
                    {
                        player->heal();
                    }
                }

            }

        }

    }

}
//--------------------------------
void Game::network()
{

    switch(netMode)
    {
        case NETMODE_NONE : break;
        case NETMODE_SERVER:
                            {
                                serveris.getData();
                            } break;
        case NETMODE_CLIENT:
                            {
                                client.getData();
                            } break;
    }

}


//---------------------------------
void Game::InitServer()
{
    netMode = NETMODE_SERVER;

    printf("Launching server on port: %d\n", NET_PORT);

    if (serveris.launch(NET_PORT))
    {
        printf("Server launched!\n");

    }
    else
    {
        printf("SERVER LAUNCH FAILED!\n");
    }
}
//--------------------------------
void Game::StopServer()
{

    int cnt = 0;
    char buffer[MAX_MESSAGE_DATA_SIZE]{};

    strcpy(buffer, NET_HEADER);
    cnt += NET_HEADER_LEN;
    buffer[cnt] = NET_SERVER_MSG_SHUTTING_DOWN;
    ++cnt;

    for (unsigned i = 0; i < serveris.clientCount(); ++i)
    {
        serveris.sendData(i, buffer, cnt);
    }

    netMode = NETMODE_NONE;
    serveris.shutDown();
}
//---------------------------------
bool Game::JoinServer(const char* ip, unsigned port)
{
    if (client.open())
    {
        char buf[MAX_MESSAGE_DATA_SIZE];
        unsigned index = 0;
        strcpy(buf, NET_HEADER);
        index += NET_HEADER_LEN;
        buf[index] = NET_CLIENT_MSG_CONNECT;

        sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(sockaddr_in));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr(ip);
        serverAddress.sin_port = htons(port);

        client.setServerAddress(serverAddress);
        client.sendData(buf, NET_HEADER_LEN + 1);

        netMode = NETMODE_CLIENT;
        return true;
    }

    return false;
}
//--------------------------
void Game::QuitServer()
{
    netMode = NETMODE_NONE;
    char buf[MAX_MESSAGE_DATA_SIZE];
    unsigned index = 0;
    strcpy(buf, NET_HEADER);
    index += NET_HEADER_LEN;
    buf[index] = NET_CLIENT_MSG_QUIT;
    client.sendData(buf, NET_HEADER_LEN + 1);
    client.shutdown();
}

//--------------------------------
void Game::SendClientDoorState(int doorx,int doory, unsigned char doorframe)
{
    int index = 0;
    char bufer[MAX_MESSAGE_DATA_SIZE];
    strcpy(bufer, NET_HEADER);
    index += NET_HEADER_LEN;
    bufer[index] = NET_CLIENT_MSG_DOOR;
    ++index;
    memcpy(&bufer[index], &doorx, sizeof(int));
    index += sizeof(int);
    memcpy(&bufer[index], &doory, sizeof(int));
    index += sizeof(int);
    memcpy(&bufer[index], &doorframe, sizeof(unsigned char));
    index += sizeof(unsigned char);
    client.sendData(bufer, index);
}
//--------------------------------
void Game::SendResurrectMessageToClient(unsigned clientIdx, unsigned playerIdx)
{
    int len = 0;
    char buffer[MAX_MESSAGE_DATA_SIZE];

    strcpy(buffer, NET_HEADER);
    len += NET_HEADER_LEN;
    buffer[len] = NET_SERVER_MSG_COOP_RESURRECT;
    ++len;
    memcpy(&buffer[len], &playerIdx, sizeof(unsigned));
    len += sizeof(unsigned);

    serveris.sendData(clientIdx, buffer, len);
}
//--------------------------------
void Game::SendResurrectMessageToServer()
{
    int len = 0;
    char buffer[MAX_MESSAGE_DATA_SIZE];

    strcpy(buffer, NET_HEADER);
    len += NET_HEADER_LEN;
    buffer[len] = NET_CLIENT_MSG_COOP_RESURRECT;
    ++len;

    client.sendData(buffer, len);

}

//--------------------------------
void Game::SendGameOverMessageToClient(unsigned clientIdx)
{
    int len = 0;
    char buffer[MAX_MESSAGE_DATA_SIZE];

    strcpy(buffer, NET_HEADER);
    len += NET_HEADER_LEN;
    buffer[len] = NET_SERVER_MSG_COOP_GAME_OVER;
    ++len;

    serveris.sendData(clientIdx, buffer, len);
}

//--------------------------------
void Game::SendServerDoorState(unsigned int clientIndex, int doorx,int doory, unsigned char doorframe)
{
    int index = 0;
    char bufer[MAX_MESSAGE_DATA_SIZE];

    strcpy(bufer, NET_HEADER);
    index += NET_HEADER_LEN;
    bufer[index] = NET_SERVER_MSG_DOOR;
    ++index;
    memcpy(&bufer[index],&doorx,sizeof(int));
    index += sizeof(int);
    memcpy(&bufer[index],&doory,sizeof(int));
    index += sizeof(int);
    memcpy(&bufer[index],&doorframe,sizeof(unsigned char));
    index += sizeof(unsigned char);

    serveris.sendData(clientIndex,bufer,index);
}

//--------------------------------
void Game::DoorsInteraction()
{

    int clientIndex = (netMode == NETMODE_CLIENT)? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);

    if (door_tim == 0)
    {
        Vector3D vec = MakeVector(20.0f, 0, player->angle);

        const int drx = round((player->x + vec.x) / 32.f);
        const int dry = round((player->y - vec.y) / 32.f);

        SoundSystem* ss = SoundSystem::getInstance();


        if (Keys[ACTION_OPEN])
        {
            //  opening
            if ((mapas.tiles[dry][drx] == 67) ||
                (mapas.tiles[dry][drx] == 65) ||
                (mapas.tiles[dry][drx] == 69) ||
                (mapas.tiles[dry][drx] == 71))
            {
                mapas.tiles[dry][drx]++;
                mapas._colide[dry][drx] = false;
                door_tim = 1;
                AdaptSoundPos(8, player->x, player->y);
                ss->playsound(8);
            }
            else if ((mapas.tiles[dry][drx] == 68) ||  //  closing
                     (mapas.tiles[dry][drx] == 66) ||
                     (mapas.tiles[dry][drx] == 70) ||
                     (mapas.tiles[dry][drx] == 72)
                    )
            {
                if (!CirclesColide(player->x, player->y, 16.0f, drx * 32.0f, dry * 32.0f, 8.0f))
                {
                    mapas.tiles[dry][drx]--;
                    mapas._colide[dry][drx] = true;
                    door_tim = 1;
                    AdaptSoundPos(9, player->x, player->y);
                    ss->playsound(9);
                }
            }


            switch(netMode)
            {
                case NETMODE_NONE: break;
                case NETMODE_SERVER:
                {
                    for (unsigned int i = 0; i < serveris.clientCount(); i++)
                    {
                        SendServerDoorState(i, drx, dry, mapas.tiles[dry][drx]);
                    }
                 } break;
                case NETMODE_CLIENT:
                {
                    SendClientDoorState(drx, dry, mapas.tiles[dry][drx]);
                }
            }

        }
    }
    else
    {
        door_tim++;

        if (door_tim == 30)
        {
            door_tim=0;
        }
    }

}
//-------------------------
void Game::HandleInteractionsWithDeadPlayers()
{
    if (netGameState == MPMODE_COOP)
    {

        if (Keys[ACTION_OPEN])
        {
            const int clientIdx = (netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0;
            Dude* player = mapas.getPlayer(clientIdx);
            Vector3D playerDirection = MakeVector(PLAYER_RADIUS, 0.f, player->angle);

            Vector3D interactionCenter(player->x + playerDirection.x, player->y + playerDirection.x, 0);

            for (unsigned i = mapas.enemyCount; i < mapas.mons.count(); ++i)
            {

                if (i != (unsigned)mapas.enemyCount + clientIdx)
                {
                    if (mapas.mons[i].shot)
                    {

                        if (CirclesColide(mapas.mons[i].x, mapas.mons[i].y, 10.f, interactionCenter.x, interactionCenter.y, 10.f))
                        {
                            mapas.mons[i].shot = false;
                            mapas.mons[i].setFrame(mapas.mons[i].activeSkin[mapas.mons[i].getCurrentWeapon()] * 4);
                            mapas.mons[i].heal();

                            if (netMode == NETMODE_SERVER)
                            {
                                SendResurrectMessageToClient(i - mapas.enemyCount - 1, i);
                            }
                            else if (netMode == NETMODE_CLIENT)
                            {
                                SendResurrectMessageToServer();
                            }

                        }

                    }
                }
            }
        }
    }
}

//-------------------------
void Game::goToEnding()
{
    gameOver = false;
    state = GAMESTATE_ENDING;
    PlayNewSong("crazy.ogg");
}


//-----------------------------
void Game::CheckForExit()
{

    int clientIndex = (netMode == NETMODE_CLIENT)? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);
    const int playerX = round(player->x / TILE_WIDTH);
    const int playerY = round(player->y / TILE_WIDTH);

    //printf("%d %d %d\n", playerY, playerX, mapas.tiles[playerY][playerX]);

    if (mapas.tiles[playerY][playerX] == TILE_EXIT)
    {
        mapai.current++;

        if (mapai.current == MAP_COUNT)
        {
            goToEnding();
        }

        int otherPlayers = (netMode == NETMODE_SERVER) ? serveris.clientCount() : 0;

        if (netMode == NETMODE_CLIENT)
        {
            SendWarpMessage();
        }
        else
        {
            GoToLevel(player->getHP(), player->ammo, mapai.current, otherPlayers); //server, offline
        }


        if (netMode == NETMODE_SERVER)
        {
            for (int a = 0; a < (int)serveris.clientCount(); ++a)
            {
                SendMapInfo(a, mapas);
                SendMapData(a, mapas);
            }

        }

    }


}

//-------
int Game::slimeReaction(int index)
{

    const int montileY = round(mapas.mons[index].y / TILE_WIDTH);
    const int montileX = round(mapas.mons[index].x / TILE_WIDTH);

    if ((!mapas.mons[index].shot)&&(!mapas.mons[index].spawn) && (!godmode) &&
        ((mapas.tiles[montileY][montileX] == TILE_SLIME) ||
        (mapas.tiles[montileY][montileX] == TILE_SLIME + 1)) &&
        (slimeTimer == 5))
    {
        mapas.mons[index].damage(SLIME_DAMAGE);
        mapas.mons[index].lastDamagedBy = DAMAGER_NOBODY;
        mapas.mons[index].hit = true;
        return SLIME_DAMAGE;
    }

    return 0;
}
//-----------------------------
void Game::SendBulletImpulse(int monsterindex, int ammo, int clientIndex, unsigned char weaponType)
{
    int index = 0;
    char buf[MAX_MESSAGE_DATA_SIZE];

    memcpy(&buf[index], NET_HEADER, NET_HEADER_LEN);
    index += NET_HEADER_LEN;
    buf[index] = NET_SERVER_MSG_WEAPON_SHOT;
    ++index;
    memcpy(&buf[index], &monsterindex, sizeof(int));
    index += sizeof(int);
    memcpy(&buf[index], &ammo, sizeof(int));
    index += sizeof(int);
    memcpy(&buf[index], &weaponType, sizeof(unsigned char));
    index += sizeof(unsigned char);

    serveris.sendData(clientIndex, buf, index);
}
//------------------------------------------
void Game::SendServerEquipedCartridgeToAllClients(unsigned dudeIdx, int equipedGame)
{

    for (unsigned i = 0; i < serveris.clientCount(); ++i)
    {
        if (dudeIdx - mapas.enemyCount == i + 1)
        {
            continue;
        }

        int len = 0;
        char buffer[MAX_MESSAGE_DATA_SIZE];
        strcpy(buffer, NET_HEADER);
        len += NET_HEADER_LEN;
        buffer[len] = NET_SERVER_MSG_GAME_EQUIPED;
        ++len;
        memcpy(&buffer[len], &dudeIdx, sizeof(unsigned));
        len += sizeof(unsigned);
        memcpy(&buffer[len], &equipedGame, sizeof(int));
        len += sizeof(int);

        serveris.sendData(i, buffer, len);

    }
}


//------------------------------------------
void Game::SendClientEquipedCartridgeIdxToServer(unsigned index)
{

    int len = 0;
    char buffer[MAX_MESSAGE_DATA_SIZE];
    strcpy(buffer, NET_HEADER);
    len += NET_HEADER_LEN;

    buffer[len] = NET_CLIENT_MSG_GAME_EQUIPED;
    ++len;
    memcpy(&buffer[len], &index, sizeof(unsigned));
    len += sizeof(unsigned);

    client.sendData(buffer, len);
}

//------------------------------------------
void Game::SendClientMeleeImpulseToServer(int victimID, int hp)
{
    char buferis[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    strcpy(buferis, NET_HEADER);
    index += NET_HEADER_LEN;
    buferis[index] = NET_CLIENT_MSG_MELEE_ATTACK;
    ++index;
    memcpy(&buferis[index], &victimID, sizeof(int));
    index += sizeof(int);
    memcpy(&buferis[index], &hp, sizeof(int));
    index += sizeof(int);
    client.sendData(buferis, index);
}
//----------------------------------------
void Game::SendClientShootImpulseToServer(WeaponTypes weaponType)
{

    int clientIndex = (netMode == NETMODE_CLIENT)? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);

    char buf[MAX_MESSAGE_DATA_SIZE];
    int pos = 0;

    strcpy(buf, NET_HEADER);
    pos += NET_HEADER_LEN;

    buf[pos] = NET_CLIENT_MSG_WEAPON_SHOT;
    ++pos;
    memcpy(&buf[pos], &(player->ammo), sizeof(int));
    pos += sizeof(int);

    unsigned char type = (unsigned char)weaponType;

    memcpy(&buf[pos], &type, sizeof(unsigned char));
    pos += sizeof(unsigned char);
    client.sendData(buf, pos);
}
//----------------------------------------
void Game::SendServerMeleeImpulseToClient(unsigned int clientIndex, int victim, int hp, int attacker)
{
    char buferis[MAX_MESSAGE_DATA_SIZE];
    int index = 0;

    memcpy(&buferis[index], NET_HEADER, NET_HEADER_LEN);
    index += NET_HEADER_LEN;
    buferis[index] = NET_SERVER_MSG_MELEE_ATTACK;
    ++index;
    memcpy(&buferis[index], &victim, sizeof(int));
    index += sizeof(int);
    memcpy(&buferis[index], &hp, sizeof(int));
    index += sizeof(int);
    memcpy(&buferis[index], &attacker, sizeof(int));
    index += sizeof(int);

    serveris.sendData(clientIndex, buferis, index);
}

//----------------------------------
void Game::BeatEnemy(int aID, int damage)
{
    if (mapas.mons[aID].canAtack)
    {
        mapas.mons[aID].shoot(false, WEAPONTYPE_REGULAR, &bulbox);
        Vector3D vec = MakeVector(16.0f, 0, mapas.mons[aID].angle);

        for (unsigned long i = 0; i < mapas.mons.count(); ++i)
        {

            if (mapas.mons[aID].hitIt(mapas.mons[i], vec.x, vec.y, damage) > -1)
            {

                switch(netMode)
                {
                    case NETMODE_NONE: break;
                    case NETMODE_SERVER:
                                       {
                                           for (unsigned clientIdx = 0; clientIdx < serveris.clientCount(); ++clientIdx)
                                           {
                                                SendServerMeleeImpulseToClient(clientIdx, i, mapas.mons[i].getHP(), mapas.mons[i].lastDamagedBy);
                                           }
                                       } break;
                    case NETMODE_CLIENT:
                                       {
                                           SendClientMeleeImpulseToServer(i, mapas.mons[i].getHP());
                                       }
                }

            }

        }
    }
}



//-------------------------
void Game::MonsterAI(int index)
{
    SoundSystem * ss = SoundSystem::getInstance();

    int linijosIlgis = 0;

    int tmpcnt = bulbox.count();

    if (mapas.mons[index].spawn)
    {
        mapas.mons[index].respawn();

        //kill entity if you spawn on it
        if (!mapas.mons[index].spawn)
        {
            for (unsigned i=0; i< mapas.mons.count(); i++)
            {
                if (i != (unsigned)index)
                {
                    if (CirclesColide(mapas.mons[index].x, mapas.mons[index].y, 10.0f, mapas.mons[i].x, mapas.mons[i].y, 8.0f))
                    {
                        if (i < (unsigned)mapas.enemyCount)
                        {
                            KillEnemy(i);
                        }
                        else
                        {
                            KillPlayer(i);
                        }
                    }
                }
            }

        }
    }

    if (!mapas.mons[index].shot && !mapas.mons[index].spawn)
    {
        bool movedFreely = mapas.mons[index].move(1.0f, 0.0f, 8.0f, mapas, (netGameState == MPMODE_COOP));

        if (!movedFreely)
        {
            mapas.mons[index].rotate(0.1f);
        }


        //1 who collides inside the shape
        int kiekplayeriu = 1;

        if (netMode == NETMODE_SERVER)
        {
            kiekplayeriu += serveris.clientCount();
        }

        int victimindex=0;

        for (int i = mapas.enemyCount; i < mapas.enemyCount + kiekplayeriu; ++i)
        {
            if ((CirclesColide(mapas.mons[i].x,mapas.mons[i].y,16,mapas.mons[index].x,mapas.mons[index].y,128))
                &&(!mapas.mons[i].shot)&&(!mapas.mons[i].spawn))
            {
                victimindex=i;
                break;
            }
        }

        if (victimindex)
        {
            //2 find the path
            Vector3D * line = Line(round(mapas.mons[index].x),
                    round(mapas.mons[index].y),
                    round(mapas.mons[victimindex].x),
                    round(mapas.mons[victimindex].y),
                    linijosIlgis, 32);

            if (linijosIlgis)
            {
                bool colide=false;
                for(int i=0; i < linijosIlgis; i++)
                {

                    unsigned ly = (unsigned)line[i].y;
                    unsigned lx = (unsigned)line[i].x;

                    if ((ly >= mapas.height()) || (lx >= mapas.width()))
                    {
                        colide = true;
                        break;
                    }

                    if (mapas.colide(lx, ly))
                    {
                        colide = true;
                        break;
                    }

                }
                if (!colide)
                {
                    mapas.mons[index].enemyseen = true;
                    mapas.mons[index].angle = atan2(mapas.mons[victimindex].y - mapas.mons[index].y,
                            mapas.mons[victimindex].x - mapas.mons[index].x);
                }
                delete []line;
            }
        }

        if ((mapas.mons[index].enemyseen))
        {
            if (mapas.mons[index].race == MONSTER_RACE_COP && !mapas.mons[index].shrinked)
            {
                mapas.mons[index].enemyseen = false;

                if (mapas.mons[index].canAtack)
                {
                    mapas.mons[index].shoot(true, WEAPONTYPE_REGULAR, &bulbox);
                    mapas.mons[index].ammo++;
                    mapas.mons[index].reloadtime = 0;

                    if (netMode == NETMODE_SERVER)
                    {
                        for (unsigned int i=0;i < serveris.clientCount(); i++)
                        {
                            SendBulletImpulse(index, mapas.mons[index].ammo, i, false);
                        }
                    }
                }
                else
                {
                    mapas.mons[index].reload(50);
                }
            }
            else if ((linijosIlgis < 2) && (linijosIlgis))
            {
                BeatEnemy(index, MONSTER_MELEE_DAMAGE);
                mapas.mons[index].enemyseen = false;

                if (!mapas.mons[index].canAtack)
                {
                    mapas.mons[index].reload(25);
                }
            }

        }


    }

    else if (mapas.mons[index].shot)
    {
        mapas.mons[index].disintegrationAnimation();

        if (!mapas.mons[index].shot) //if finally disintegrated
        {
            mapas.mons[index].initMonsterHP();
            mapas.mons[index].appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
        }
    }


    if ((!mapas.mons[index].shot) && (!mapas.mons[index].spawn))
    {

        if (mapas.mons[index].hit)
        {
            mapas.mons[index].damageAnim();
            if (!mapas.mons[index].hit)
            {
                AdaptSoundPos(11,mapas.mons[index].x,mapas.mons[index].y);
                ss->playsound(11);
            }
        }

        //eats items

        if (mapas.mons[index].race == MONSTER_RACE_GOBBLER)
        {
            for (unsigned i=0; i < mapas.items.count(); i++)
            {
                if (!mapas.mons[index].item)
                {
                    if (CirclesColide(mapas.items[i].x, mapas.items[i].y, 8.0f, mapas.mons[index].x,mapas.mons[index].y,16.0f))
                    {
                        mapas.mons[index].item = mapas.items[i].value;
                        AdaptSoundPos(5,mapas.items[i].x,mapas.items[i].y);
                        ss->playsound(5);

                        if (netMode == NETMODE_SERVER)
                        {
                            for (unsigned a = 0; a < serveris.clientCount(); ++a)
                            {
                                SendItemSRemove(i, a, false);
                            }
                        }

                        mapas.removeItem(i);
                    }
                }

            }

        }

    }

    if (bulbox.count()>tmpcnt)
    {
        AdaptSoundPos(0,mapas.mons[index].x,mapas.mons[index].y);
        ss->playsound(0);
    }

}
//-------------------------------
void Game::AnimateSlime()
{
    if (mapas.tiles)
    {
        slimeTimer++;

        if (slimeTimer > 25)
        {
            slimeTimer = 0;

            if (!slimeswap)
            {
                mapas.ReplaceTiles(TILE_SLIME, TILE_SLIME + 1);
                slimeswap = true;
            }
            else
            {
                mapas.ReplaceTiles(TILE_SLIME + 1, TILE_SLIME);
                slimeswap = false;
            }
        }
    }
}
//-------------------------------------------------------
void Game::GenerateTheMap(int level, int currentHp, int currentAmmo)
{
    bulbox.destroy();
    mapas.destroy();

    mapas.generate(level);

    Dude thePlayer;
    mapas.addMonster(thePlayer);
    Dude* player = mapas.getPlayer();

    player->x = (float)mapas.start.x;
    player->y = (float)mapas.start.y;
    player->id = mapas.enemyCount;
    player->shot = false;
    player->setHP(currentHp);
    player->race = MONSTER_RACE_PLAYER;
    player->ammo = currentAmmo;
    player->setWeaponCount(PLAYER_SIMULTANEOUS_WEAPONS);
    player->setSkinCount(PLAYER_MAX_SKIN_COUNT);
    player->setFrame((player->activeSkin[player->getCurrentWeapon()] + 1) * 4 - 2);


    for (int i = 0; i < PlayerCount() - 1; ++i)
    {
        Dude p;
        p.race = MONSTER_RACE_PLAYER;
        p.appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
        p.id = mapas.mons[mapas.mons.count() - 1].id + 1;
        mapas.mons.add(p);

    }


    AdaptMapView();
}


//-------------------------------------------------------
void Game::LoadTheMap(const char* name, bool createItems, int otherPlayers, int currentHp)
{
    bulbox.destroy();
    mapas.destroy();

    if (!mapas.load(name, createItems, otherPlayers))
    {
        mapas.destroy();
        mapai.Destroy();
        printf("Can't find first map!\n");
        Works = false;
    }

    if (netGameState != MPMODE_DEATHMATCH)
    {
        mapas.tiles[(int)mapas.exit.y][(int)mapas.exit.x] = TILE_EXIT;
    }

    timeleft = mapas.timeToComplete;

    Dude* player = mapas.getPlayer();

    if (netMode == NETMODE_NONE)
    {
        player->x = (float)mapas.start.x;
        player->y = (float)mapas.start.y;
    }
    else
    {
        player->appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
    }

    player->setHP(currentHp);
    player->id = mapas.enemyCount;
    AdaptMapView();
}
//-------------------------------------------------------
void Game::LoadFirstMap()
{
    if (netMode != NETMODE_CLIENT) //offline & server
    {
        GenerateTheMap(0, ENTITY_INITIAL_HP, ENTITY_INITIAL_AMMO);
    }
}

//-------------------------------------
void Game::ResetVolume()
{

    SoundSystem* ss = SoundSystem::getInstance();

    for (unsigned i = 0; i < maxwavs; i++)
    {
        ss->setVolume(i, sys.soundFXVolume);
    }

    music.setVolume(sys.musicVolume);
}


//-------------------------------------------------------------
void Game::TitleMenuLogic()
{

    if (cartridgeCollection.active())
    {

        cartridgeCollection.getInput(Keys, OldKeys);

        if (cartridgeCollection.isCanceled())
        {
            cartridgeCollection.deactivate();
            cartridgeCollection.reset();
            mainmenu.activate();
        }
    }


    if (mainmenu.active())
    {
        switch(netMode)
        {
            case NETMODE_NONE: break;
            case NETMODE_SERVER: StopServer(); break;
            case NETMODE_CLIENT: QuitServer(); break;
        }

        if (!mainmenu.selected)
        {
            mainmenu.getInput(Keys, OldKeys);
        }
        else
        {
            switch(mainmenu.state)
            {
                case 0:
                    {
                        mainmenu.reset();
                        state = GAMESTATE_INTRO;
                        PlayNewSong("crazy.ogg");   
                    } break;
                case 1:
                    {
                        netmenu.activate();
                        mainmenu.reset();
                        mainmenu.deactivate();
                    }break;
                case 2:
                    {
                        cartridgeCollection.activate();
                        mainmenu.reset();
                        mainmenu.deactivate();
                    } break;
                case 3:
                    {
                        options.activate();
                        mainmenu.reset();
                        mainmenu.deactivate();
                    }break;
                case 4:
                    {
                        Works = false;
                        mainmenu.reset();
                    }break;
            }
        }
    }

    if (netmenu.active())
    {
        if (!netmenu.selected)
        {
            netmenu.getInput(Keys, OldKeys);
        }
        else{
            switch(netmenu.state)
            {
                case 0:{
                    netgame.activate();
                    netmenu.deactivate();
                    netmenu.reset();
                } break;

                case 1:
                {
                    if (netmenu.active())
                        netmenu.deactivate();
                    netmenu.reset();
                    ipedit.activate();
                    EditText[0] = 0;
                }
            }
        }


        if (netmenu.canceled){
            mainmenu.activate();
            netmenu.reset();
            netmenu.deactivate();
        }
    }

        //-------IP EDIT    
        if (ipedit.active())
        {
            if (!ipedit.entered)
            {
                ipedit.getInput(EditText, globalKEY);
            }
            else
            {
                netmenu.reset();
                PlayNewSong("music.ogg");
                JoinServer(ipedit.text, NET_PORT);
                ipedit.deactivate();
                ipedit.reset();
            }

            if (ipedit.canceled)
            {
                ipedit.deactivate();
                ipedit.reset();
                netmenu.activate();
            }

        }
    //----------------
        if (netgame.active())
        {
            if (!netgame.selected)
            {
                netgame.getInput(Keys, OldKeys);

                if (netgame.canceled)
                {
                    netmenu.activate();
                    netgame.reset();
                    netgame.deactivate();
                }

            }
            else
            {
                if (netgame.state == 1)
                {
                    netGameState = MPMODE_DEATHMATCH;
                    fragTable.destroy();
                    fragTable.add(0);
                    mapmenu.activate();
                    netgame.reset();
                    netgame.deactivate();
                }
                else if (netgame.state == 0)
                {
                    netGameState = MPMODE_COOP;
                    state = GAMESTATE_GAME;
                    netgame.reset();
                    netgame.deactivate();
                    PlayNewSong("music.ogg");
                    InitServer();
                    LoadFirstMap();
                }
            }
        }

        if (mapmenu.active())
        {
            if (!mapmenu.selected)
            {
                mapmenu.getInput(Keys, OldKeys);

                if (mapmenu.canceled)
                {
                    netgame.activate();
                    mapmenu.reset();
                    mapmenu.deactivate();
                }

            }
            else
            {
                mapai.current = mapmenu.state;
                state = GAMESTATE_GAME;
                PlayNewSong("music.ogg");
                mapmenu.reset();
                mapmenu.deactivate();
                InitServer();
                GoToLevel(ENTITY_INITIAL_HP, ENTITY_INITIAL_AMMO, mapai.current, serveris.clientCount());
            }
        }

        if (options.active())
        {
            if (!options.selected)
            {
                options.getInput(Keys, OldKeys);
            }
            else
            {
                options.deactivate();
                //mainmenu.activate();
                switch (options.state){
                    case 0:{
                        options.reset();
                        MusicVolumeC.activate();
                           } break;
                    case 1:{
                            options.reset();
                            SfxVolumeC.activate();
                           } break;
                }
            }
            if (options.canceled){
                mainmenu.activate();
                options.reset();
                options.deactivate();
            }
        }


        if (MusicVolumeC.active())
        {
            if (!MusicVolumeC.selected)
            {
                MusicVolumeC.getInput(Keys, OldKeys);
            }
            else{
                MusicVolumeC.deactivate();
                options.activate();
                sys.musicVolume = MusicVolumeC.state / 1000.f;
                printf("new music volume %f\n", sys.musicVolume);
                ResetVolume();
                MusicVolumeC.reset();
                char buf[1024];
                printf("Document path: %s\n", DocumentPath);
                sprintf(buf, "%s/settings.cfg", DocumentPath);
                sys.write(buf);
            }

            if (MusicVolumeC.canceled)
            {
                MusicVolumeC.deactivate();
                options.activate();
                MusicVolumeC.reset();
            }

        }

        if (SfxVolumeC.active())
        {
            if (!SfxVolumeC.selected)
                SfxVolumeC.getInput(Keys, OldKeys);
            else{
                SfxVolumeC.deactivate();
                options.activate();
                sys.soundFXVolume=SfxVolumeC.state*-1;
                ResetVolume();
                SfxVolumeC.reset();

            }

            if (SfxVolumeC.canceled){
                SfxVolumeC.deactivate();
                options.activate();
                SfxVolumeC.reset();

            }

        }


}
//---------------------------------------------------------
void Game::IntroScreenLogic()
{
    if ((Keys[ACTION_OPEN] && !OldKeys[ACTION_OPEN]) || (!FirstTime))
    {
        state = GAMESTATE_HELP;
        intro.reset();
    }

    intro.logic();
}
//---------------------------------------------------------------
void Game::HelpScreenLogic()
{
    itmtim++;

    if (itmtim > 10)
    {
        mapas.itmframe++; //supposed to be animated items
        if (mapas.itmframe > 3)
        {
            mapas.itmframe = 0;
        }

        itmtim = 0;
    }

    if ((Keys[ACTION_OPEN] && !OldKeys[ACTION_OPEN])||(!FirstTime))
    {

        if (FirstTime)
        {
            FirstTime = false;
        }

        state = GAMESTATE_GAME;

        PlayNewSong("music.ogg");
        LoadFirstMap();

    }

}
//-----------------------------
void Game::EndingLogic()
{
    itmtim++;

    if (itmtim > 10)
    {
        mapas.itmframe++; //animated items ?
        if (mapas.itmframe > 3)
        {
            mapas.itmframe = 0;
        }

        itmtim=0;
    }


    if (Keys[ACTION_OPEN] && !OldKeys[ACTION_OPEN])
    {
        state = GAMESTATE_TITLE;
        intro.reset();

        for (unsigned i = 0; i < loot.count(); ++i)
        {
            stash.add(loot[i]);
        }

        loot.destroy();

        mapai.current = 0;
        FirstTime = true;
        mainmenu.activate();
        PlayNewSong("evil.ogg");

    }

}

//------------------------------
int Game::PlayerCount()
{
    int players = 1;

    switch(netMode)
    {
        case NETMODE_NONE: break;
        case NETMODE_SERVER: players += serveris.clientCount(); break;
        case NETMODE_CLIENT: players += otherClientCount; break;
    }

    return players;
}

//===============================================================

void Game::logic()
{

    if ((mapas.timeToComplete) && (state == GAMESTATE_GAME))
    {
        ms += 10;

        if (ms >= 1000)
        {
            if (timeleft)
            {
                timeleft--;
            }
            else
            {
                if (netGameState == MPMODE_DEATHMATCH)
                {
                    if (netMode == NETMODE_SERVER)
                    {
                        StopServer();
                        goToEnding();
                        doFadein = true;
                        fadeTimer = 0;
                        objectivetim = 200;
                        gameOver = false;
                    }
                }
                else
                {
                    int idx = (netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0;
                    KillPlayer(mapas.enemyCount + idx);
                }
            }
            ms = 0;
        }
    }



    switch (netMode)
    {
        case NETMODE_NONE: break;
        case NETMODE_SERVER:
        {
            if (serveris.isRunning())
            {
                ++clientInfoSendCounter;

                if (clientInfoSendCounter > SERVER_SKIP_SENDING_POS_FRAMES) // we don't need to send creature coords every frame
                {

                    for (int a = 0; a < (int)serveris.clientCount(); ++a)
                    {
                        char buf[MAX_MESSAGE_DATA_SIZE];
                        memcpy(&buf, NET_HEADER, NET_HEADER_LEN);
                        buf[NET_HEADER_LEN] = NET_SERVER_MSG_PING;
                        serveris.sendData(a, buf, NET_HEADER_LEN + 1);

                        SendPlayerInfoToClient(a);
                    }

                    clientInfoSendCounter = 0;
                }


                ParseMessagesServerGot();
            }
        } break;
        case NETMODE_CLIENT:
        {
            if (client.isOpen())
            {
                if (Client_GotMapData)
                {
                    SendClientCoords();
                }

                ParseMessagesClientGot();
            }
        }
    }

    if (music.playing())
    {
        music.update();
    }


    switch(state)
    {
        case GAMESTATE_TITLE  : TitleMenuLogic();   break;
        case GAMESTATE_INTRO  : IntroScreenLogic(); break;
        case GAMESTATE_HELP   : HelpScreenLogic();  break;
        case GAMESTATE_ENDING : EndingLogic();      break;
        case GAMESTATE_GAME   : CoreGameLogic();    break;
    }

    if (doFadein)
    {
        fadeTimer++;

        if (fadeTimer == MAX_FADE_TIMER_VAL)
        {
            doFadein = false;
            fadeTimer = 0;
        }

    }

    touches.up.destroy();
    touches.down.destroy();
    touches.move.destroy();

    OldMouseX = MouseX;
    OldMouseY = MouseY;

    OldGamepadLAxis = gamepadLAxis;
    OldGamepadRAxis = gamepadRAxis;

}
//------------------------------------
void Game::HandlePlayerAttacks(Dude* player, int clientIndex)
{
    if (!player->canAtack)
    {
        return;
    }

    SoundSystem* ss = SoundSystem::getInstance();

    bool stillHaveAmmo = false;

    const int currentWeapon = player->getCurrentWeapon();


    WeaponTypes weaponType = WEAPONTYPE_REGULAR;

    switch(currentWeapon)
    {
        case 1:
            {

                if (player->equipedGame == ITEM_GAME_UNABOMBER_GUY)
                {
                    weaponType = WEAPONTYPE_MINES;

                    stillHaveAmmo = player->shoot(true, WEAPONTYPE_MINES, &bulbox);

                    if (stillHaveAmmo)
                    {
                        PlaySoundAt(ss, player->x, player->y, 12);
                    }

                }
                else if (player->equipedGame == ITEM_GAME_DUKE_ATOMIC)
                {
                    weaponType = WEAPONTYPE_SHRINKER;
                    stillHaveAmmo = player->shoot(true, WEAPONTYPE_SHRINKER, &bulbox);

                    if (stillHaveAmmo)
                    {
                        PlaySoundAt(ss, player->x, player->y, 13);
                    }


                }
                else if (player->equipedGame == ITEM_GAME_CONTRABANDISTS)
                {
                    weaponType = WEAPONTYPE_SPREAD;
                    stillHaveAmmo = player->shoot(true, weaponType, &bulbox);

                    if (stillHaveAmmo)
                    {
                        PlaySoundAt(ss, player->x, player->y, 0);
                    }

                }
                else
                {

                    weaponType = WEAPONTYPE_REGULAR;

                    stillHaveAmmo = player->shoot(true, WEAPONTYPE_REGULAR, &bulbox);

                    if (stillHaveAmmo)
                    {
                        PlaySoundAt(ss, player->x, player->y, 0);
                    }
                }
            } break;


        case 0: BeatEnemy(mapas.enemyCount + clientIndex, PLAYER_MELEE_DAMAGE); break;
    }

    if (currentWeapon > 0)
    {
        if (stillHaveAmmo)
        {
            if (netMode == NETMODE_CLIENT)
            {
                SendClientShootImpulseToServer(weaponType);
            }


            if (netMode == NETMODE_SERVER)
            {
                for (unsigned i = 0; i < serveris.clientCount(); i++)
                {
                    SendBulletImpulse(mapas.enemyCount + clientIndex, player->ammo, i, weaponType);
                }
            }
        }
        else
        {
            if (noAmmo)
            {
                player->chageNextWeapon();
                noAmmo = false;
            }
            else
            {
                PlaySoundAt(ss, player->x, player->y, 4);

                if (!noAmmo)
                {
                    noAmmo = true;
                }
            }
        }
    }

}


//------------------------------------
void Game::CoreGameLogic()
{

    int clientIndex = (netMode == NETMODE_CLIENT) ? (clientMyIndex + 1) : 0;
    Dude* player = mapas.getPlayer(clientIndex);


    if (!inventory.active())
    {
        if (Keys[ACTION_MAP] && !OldKeys[ACTION_MAP])
        {
            showMiniMap = !showMiniMap;
        }
    }

    if (inventory.active())
    {
        inventory.getInput(Keys, OldKeys, loot);

        if (inventory.isCanceled())
        {
            inventory.deactivate();
            inventory.reset();
        }

        if (inventory.isSelected())
        {
            inventory.deactivate();
            inventory.reset();
            if (player->equipedGame)
            {
                loot.add(player->equipedGame);
            }


            if (netMode == NETMODE_SERVER)
            {
                SendServerEquipedCartridgeToAllClients(mapas.enemyCount, loot[inventory.getSelected()]);
            }
            else if (netMode == NETMODE_CLIENT)
            {
                SendClientEquipedCartridgeIdxToServer(inventory.getSelected());
            }

            equipCartridge(player, loot[inventory.getSelected()]);

            loot.remove(inventory.getSelected());
        }
    }

    if (Keys[ACTION_INVENTORY] && !OldKeys[ACTION_INVENTORY] && !gameOver)
    {
        inventory.activate();
    }

    mapas.fadeDecals();

    if (objectivetim)  //  decrement objective timer
    {
        objectivetim--;
    }

    itmtim++;

    if (itmtim > 10)
    {
        mapas.itmframe++;  //  item animation

        if (mapas.itmframe > 3)
        {
            mapas.itmframe = 0;
        }

        itmtim=0;
    }


    if (gameOver)
    {
        if (Keys[ACTION_OPEN] && !OldKeys[ACTION_OPEN])
        {
            goToEnding();
            doFadein = true;
            fadeTimer = 0;
            objectivetim = 200;
            gameOver = false;
            return;
        }
    }


    AnimateSlime();

    //hero movement

    if (player->shot)
    { //hero dies here

        if (netMode != NETMODE_NONE)
        {
            if (netGameState == MPMODE_DEATHMATCH)
            {
                player->disintegrationAnimation();
            }
            else if (netGameState == MPMODE_COOP)
            {
                if (netMode == NETMODE_SERVER)
                {
                    bool foundAlive = false;

                    for (unsigned i = 0; i < serveris.clientCount(); ++i)
                    {
                        if (mapas.mons[mapas.enemyCount + 1 + i].shot == false)
                        {
                            foundAlive = true;
                            break;
                        }
                    }

                    if (foundAlive == false && !gameOver)
                    {
                        gameOver = true;

                        for (unsigned i = 0; i < serveris.clientCount(); ++i)
                        {
                            SendGameOverMessageToClient(i);
                        }
                    }

                }
            }
        }
        else  // singleplayer
        {
            if (!gameOver)
            {
                gameOver = true;
            }
        }

        if (!player->shot) // disintegration has ended
        {

            if (netMode == NETMODE_NONE)
            {
                player->x = (float)mapas.start.x;
                player->y = (float)mapas.start.y;
            }
            else
            {
                player->appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
            }

            AdaptMapView();
        }

    }



    if (player->spawn)
    { //respawn
        player->respawn();

        if ((player->ammo < 1) && (!player->spawn))
        {
            player->ammo = ENTITY_INITIAL_AMMO;
        }
    }



    if (((OldGamepadRAxis.x != gamepadRAxis.x ) || (OldGamepadRAxis.y != gamepadRAxis.y )) &&
        !((int)gamepadRAxis.x == 0 && (int)gamepadRAxis.y == 0)&& (!player->shot) && (!player->spawn))
    {
        Vector3D dir = gamepadRAxis;

        player->angle = M_PI / 2.f - atan2(dir.x, dir.y);
    }


    if ((((int)MouseX != (int)OldMouseX) || ((int)MouseY != (int)OldMouseY)) &&
            (!player->shot) && (!player->spawn))
    {

        Vector3D dudePosOnScreen(player->x + mapas.getPos().x,
                                 player->y + mapas.getPos().y, 0);

        Vector3D mouse(MouseX, MouseY, 0);

        Vector3D fv = mouse - dudePosOnScreen;
        fv.normalize();
        float newAngle = M_PI / 2 - atan2(fv.x, fv.y);
        player->angle = newAngle;
    }

    if (!player->shot && !player->spawn && player->canAtack && !inventory.active())
    {

        if ((Keys[ACTION_NEXT_WEAPON]) && (!Keys[ACTION_FIRE]) && (!OldKeys[ACTION_NEXT_WEAPON]))
        {
            player->chageNextWeapon();
        }

        Vector3D mov = Vector3D(gamepadLAxis.x, -gamepadLAxis.y, 0);
        mov.normalize();

        bool movedWithGamepad = false;

        if (fabs(mov.x) > 0.5f || fabs(mov.y) > 0.5f)
        {
            movedWithGamepad = true;
        }

        if (player->equipedGame == ITEM_GAME_SPEEDBALL)
        {
            mov.x = mov.x * ENTITY_SPEEDBALL_SPEED;
            mov.y = mov.y * ENTITY_SPEEDBALL_SPEED;
        }

        player->moveGamePad(mov, PLAYER_RADIUS, mapas, (netGameState == MPMODE_COOP));
        AdaptMapView();

        if ((Keys[0] || Keys[1] || Keys[2] || Keys[3]) && !movedWithGamepad)
        {
            MoveDude();
        }
    }

    SoundSystem* ss = SoundSystem::getInstance();
    ss->setupListener(Vector3D(player->x, 0, player->y).v, Vector3D(player->x, 0, player->y).v);

    ItemPickup();


    if (touches.allfingersup)
    {
        if (touches.up.count())
        {
            Keys[ACTION_FIRE] = 1;
        }
    }


    //shooting
    if ((Keys[ACTION_FIRE]) && (!player->shot) && (!player->spawn) && (!inventory.active()))
    {
        HandlePlayerAttacks(player, clientIndex);
    }
    //--
    if (!player->canAtack)
    {
        player->reload(25);
    }


    for (int i = mapas.enemyCount; i < mapas.enemyCount + PlayerCount(); ++i)
    {

        mapas.mons[i].update();

        if (mapas.mons[i].hit)
        {
            if (i == mapas.enemyCount + clientIndex)
            {
                doRumble = true;
            }

            mapas.mons[i].damageAnim();

            if (!mapas.mons[i].hit)
            {
                PlaySoundAt(ss, mapas.mons[i].x, mapas.mons[i].y, 10);
            }
        }
    }


    if (netMode == NETMODE_NONE || netMode == NETMODE_SERVER)
    {
        for (unsigned i = mapas.enemyCount; i < mapas.mons.count(); ++i)
        {
            const int dmg = slimeReaction(i); // hero reaction to slime


            if (i > (unsigned)mapas.enemyCount && dmg)
            {
                SendServerMeleeImpulseToClient(i - mapas.enemyCount - 1, i, mapas.mons[i].getHP(), DAMAGER_NOBODY);
            }
        }
    }

    HandleInteractionsWithDeadPlayers();

    DoorsInteraction();  //open doors

    CheckForExit();

    bulbox.update((const bool**)mapas._colide, mapas.mons, mapas.width(), mapas.height());


    if (netMode != NETMODE_CLIENT) // offline & server
    {

        for (int i = 0; i < mapas.enemyCount; ++i)
        {
            MonsterAI(i);
        }

        //let's kill players with 0 hp

        for (unsigned i = 0; i < mapas.mons.count(); ++i)
        {
            mapas.mons[i].killShrinked(mapas.mons, i);

            if (mapas.mons[i].equipedGame == ITEM_GAME_FART_NIGHT)
            {

                for (unsigned a = 0; a < mapas.mons.count(); ++a)
                {
                    if (i != a && !mapas.mons[a].shot && !mapas.mons[a].spawn && !mapas.mons[a].hit)
                    {
                        if (CirclesColide(mapas.mons[i].x, mapas.mons[i].y, FARTNIGHT_RADIUS, 
                                          mapas.mons[a].x, mapas.mons[a].y, 10))
                        {
                            mapas.mons[a].damage(FARTNIGHT_DAMAGE);
                            mapas.mons[a].lastDamagedBy = mapas.mons[i].id;
                            mapas.mons[a].hit = true;

                            if ( a > (unsigned)mapas.enemyCount)
                            {
                                SendServerMeleeImpulseToClient(a - mapas.enemyCount - 1, a, mapas.mons[a].getHP(), mapas.mons[i].id);
                            }
                        }
                    }
                }
            }


            if ((mapas.mons[i].getHP() <= 0) && (!mapas.mons[i].shot))
            {
                KillEnemy(i);

                if (netMode == NETMODE_SERVER)
                {

                    if (i >= (unsigned)mapas.enemyCount) //players
                    {
                        int lastDamagerID = mapas.mons[i].lastDamagedBy;
                        int lastDamagerIdx = mapas.findCreatureById(mapas.mons[i].lastDamagedBy);

                        printf("Last damager id: %d idx: %d\n", lastDamagerID, lastDamagerIdx);

                        if (lastDamagerIdx >= mapas.enemyCount)
                        {
                            int clientWithFrag = lastDamagerID - mapas.enemyCount;
                            ++fragTable[clientWithFrag];

                            if (lastDamagerIdx == mapas.enemyCount)
                            {
                                ++frags;
                            }
                            else
                            {
                                SendFragsToClient(lastDamagerIdx - mapas.enemyCount - 1, fragTable[clientWithFrag]);
                            }
                        }

                    }

                    for (unsigned clientIdx = 0; clientIdx < serveris.clientCount(); ++clientIdx)
                    {
                        SendKillCommandToClient(clientIdx, i);
                    }
                }
            }
        }
    }

}


//------------------------------------
void Game::DrawHelp()
{

    const int ICON_POS = 20;

    pics.draw(13, 320, 180, 0, true);
    WriteShadedText(130, 40, pics, PICTURE_FONT, "Colect these:");

    pics.draw(11, 150, 60, mapas.itmframe, false);
    pics.draw(11, 200, 60, mapas.itmframe + 4, false);
    pics.draw(11, 250, 60, mapas.itmframe + 8, false);

    pics.draw(7, ICON_POS, 110, 0, false);
    pics.draw(7, ICON_POS, 130, 1, false);
    WriteShadedText(ICON_POS + 40, 120, pics, PICTURE_FONT, "Ammo");
    WriteShadedText(ICON_POS + 40, 140, pics, PICTURE_FONT, "Health Up");

    WriteShadedText(ICON_POS + 40, 220, pics, PICTURE_FONT, "Controls:");
    WriteShadedText(ICON_POS + 40, 240, pics, PICTURE_FONT, "Aim with the mouse, and move with arrows");
    WriteShadedText(ICON_POS + 40, 255, pics, PICTURE_FONT, "Tab: minimap");
    WriteShadedText(ICON_POS + 40, 270, pics, PICTURE_FONT, "CTRL: fire");
    WriteShadedText(ICON_POS + 40, 285, pics, PICTURE_FONT, "SPACE: opens door");

    int monframe = mapas.itmframe;

    if (monframe == 3)
    {
        monframe = 0;
    }

    pics.draw(3, ICON_POS, 170, monframe, false);
    WriteShadedText(ICON_POS + 40, 170, pics, PICTURE_FONT, "These monsters can eat items");
    WriteShadedText(ICON_POS + 40, 190, pics, PICTURE_FONT, "kill them to retrieve items back.");

    WriteShadedText(20, sys.ScreenHeight - 20, pics, PICTURE_FONT, "hit RETURN to play...");


}

//---------------------------
void Game::DrawMissionObjectives()
{
    char buf[50];

    sprintf(buf, "Time remaining:%d:%d", mapas.timeToComplete / 60, mapas.timeToComplete - 60 * (mapas.timeToComplete / 60));
    WriteText(sys.ScreenWidth / 2 - 100, 
              sys.ScreenHeight / 2 + 50,
              pics,
              10,
              buf,
              1.f, 1.f,
              COLOR(1.f, 0.f, 0.f, 1.f),
              COLOR(1.f, 0.f, 0.f, 1.f)
              );

}

//---------------------------

void Game::render(bool useVulkan)
{
    FlatMatrix identity;
    MatrixIdentity(identity.m);

    FlatMatrix finalM = identity * OrthoMatrix;
    defaultShader.use(vkCmd);

    if (!useVulkan)
    {
        int MatrixID = defaultShader.getUniformID("ModelViewProjection");
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, finalM.m);
    }
    else // VULKAN
    {
        vkCmdPushConstants(*vkCmd,
                           *defaultShader.getVkPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(finalM.m), &finalM.m);
    }

    colorShader.use(vkCmd);

    if (!useVulkan)
    {
        int MatrixIDColor = colorShader.getUniformID("ModelViewProjection");
        glUniformMatrix4fv(MatrixIDColor, 1, GL_FALSE, finalM.m);
    }
    else //VULKAN
    {
        vkCmdPushConstants(*vkCmd,
                           *colorShader.getVkPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(finalM.m), &finalM.m);

    }

    switch(state)
    {
        case GAMESTATE_TITLE  : DrawTitleScreen();      break;
        case GAMESTATE_INTRO  : intro.draw(pics, sys);  break;
        case GAMESTATE_HELP   : DrawHelp();             break;
        case GAMESTATE_ENDING : DrawEndScreen();        break;
        case GAMESTATE_GAME   : DrawGameplay();         break;
    }

    pics.drawBatch(&colorShader, &defaultShader, 666, useVulkan, vkCmd, vulkanDevice);

}
//-------------------------------------
void Game::DrawTitleScreen()
{
    pics.draw(0, 320, 180, 0, true);
    pics.draw(16, 0, 0, 0, false, 1.f, 1.f, 0.f, COLOR(1.f, 1, 1, 1.f), COLOR(1.f, 1.f, 1.f, 1.f));

    char buf[80];

    sprintf(buf,"Jrs%dul",0);
    WriteText(sys.ScreenWidth - 50, 10, pics, 10, buf, 0.5f, 0.5f);
    sprintf(buf,"%d",2025);
    WriteText(sys.ScreenWidth - 40, 25, pics, 10, buf, 0.5f, 0.5f);

    if (mainmenu.active())
    {
        mainmenu.draw(pics,
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga"));
    }

    if (netmenu.active())
    {
        netmenu.draw(pics,
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga")
                );
    }

    if (netgame.active())
    {
        netgame.draw(pics,
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga")
                );
    }

    if (mapmenu.active())
    {
        mapmenu.draw(pics,
                     pics.findByName("pics/pointer.tga"),
                     pics.findByName("pics/charai.tga")
                    );
    }

    if (options.active())
    {
        options.draw(pics,
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga")
                );
    }

    if (ipedit.active())
    {
        ipedit.draw(pics, pics.findByName("pics/charai.tga"));
    }

    if (SfxVolumeC.active())
    {
        SfxVolumeC.draw(pics, 
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga")
                );
    }

    if (MusicVolumeC.active())
    {
        MusicVolumeC.draw(pics,
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/pointer.tga"),
                pics.findByName("pics/charai.tga")
                );
    }

    if (cartridgeCollection.active())
    {
        cartridgeCollection.draw(pics);
    }
}
//------------------------------------
void Game::DrawEndScreen()
{
    pics.draw(14, 320, 180, 0, true);
    char levelstr[255];

    if (netGameState == MPMODE_DEATHMATCH)
    {
        sprintf(levelstr, "your frags: %d", frags);
        WriteShadedText(20, 40, pics, 10, levelstr);

    }
    else
    {
        sprintf(levelstr, "Levels completed: %d", mapai.current);
        WriteShadedText(20, 40, pics, 10, levelstr);
        WriteShadedText(20, 80, pics, 10, "Your loot:");


        int itemY = 100;
        int itemX = 20;

        for (unsigned i = 0; i < loot.count(); ++i)
        {
            pics.draw(11, itemX, itemY, loot[i] - ITEM_GAME_NINJA_MAN, false);

            itemX += 32;

            if (itemX > sys.ScreenWidth - 20)
            {
                itemY += 32;
                itemX = 20;
            }

        }
    }

    WriteShadedText(260, sys.ScreenHeight - 30, pics, 10, "Press SPACE to continue...");
}
//------------------------------------
void Game::DrawGameplay()
{
    if ((mapas.width() > 0) && (mapas.height() > 0))
    {

        if (doFadein)
        {
            const float fadeColor = fadeTimer / (MAX_FADE_TIMER_VAL * 1.f);
            DrawMap(fadeColor, fadeColor, fadeColor);
        }
        else
        {
            DrawMap();
        }


    }

    if (doFadein)
    {
        WriteText(sys.ScreenWidth / 2-150, 
                sys.ScreenHeight/2-64,
                pics, 10, "Get Ready!", 2,2);
    }

    if (objectivetim && netGameState == MPMODE_COOP)
    {
        DrawMissionObjectives();
    }

    if (mapas.mons.count())
    {
        DrawStats();
    }

    if (inventory.active())
    {
        inventory.draw(pics, loot, gameData);
    }


    pics.draw(18, MouseX, MouseY, 0, true);

    if (showMiniMap)
    {
        DrawMiniMap(sys.ScreenWidth - mapas.width() * 4, sys.ScreenHeight - mapas.height() * 4);
    }

    if (gameOver)
    {
        pics.draw(19, sys.ScreenWidth / 2, sys.ScreenHeight / 2, 0, true);
    }



    if (DebugMode == 1)
    {
        DrawSomeText();
    }
}



//----------------------------------------
void Game::SendClientCoords()
{
    Dude* player = mapas.getPlayer(clientMyIndex + 1);

    char coords[MAX_MESSAGE_DATA_SIZE];
    int cnt = 0;

    strcpy(coords, NET_HEADER);
    cnt += NET_HEADER_LEN;

    coords[cnt] = NET_CLIENT_MSG_CHARACTER_DATA;
    ++cnt;
    memcpy(&coords[cnt], &(player->x), sizeof(float));
    cnt += sizeof(float);
    memcpy(&coords[cnt], &(player->y), sizeof(float));
    cnt += sizeof(float);
    memcpy(&coords[cnt], &(player->angle), sizeof(float));
    cnt += sizeof(float);
    unsigned char theFrame = player->getFrame();
    memcpy(&coords[cnt], &theFrame, sizeof(unsigned char));
    cnt += sizeof(unsigned char);
    unsigned char stats=0x0;

    if (player->shot)
    {
        stats|=0x80;
    }

    if (player->spawn)
    {
        stats|=0x40;
    }

    coords[cnt]=stats;
    cnt++;

    client.sendData(coords, cnt);
}
//-----------------------------------------
void Game::SendPlayerInfoToClient(int clientindex)
{
    unsigned char z = 0;
    char coords[MAX_MESSAGE_DATA_SIZE];
    int cnt = 0;

    for (int i = 0; i < mapas.enemyCount+(int)serveris.clientCount() + 1; i++)
    {
        if (i - mapas.enemyCount - 1 != clientindex)
        {
            z = i;

            memcpy(&coords[cnt], NET_HEADER, NET_HEADER_LEN);
            cnt += NET_HEADER_LEN;

            coords[cnt] = NET_SERVER_MSG_CHARACTER_DATA;
            ++cnt;
            memcpy(&coords[cnt],&z,sizeof(unsigned char));
            cnt += sizeof(unsigned char);
            memcpy(&coords[cnt],&mapas.mons[i].x, sizeof(float));
            cnt += sizeof(float);
            memcpy(&coords[cnt],&mapas.mons[i].y, sizeof(float));
            cnt += sizeof(float);
            memcpy(&coords[cnt],&mapas.mons[i].angle, sizeof(float));
            cnt += sizeof(float);
            unsigned char theFrame = mapas.mons[i].getFrame();
            memcpy(&coords[cnt],&theFrame, sizeof(unsigned char));
            cnt += sizeof(unsigned char);
            unsigned char stats = 0x0;

            if (mapas.mons[i].shot)
            {
                stats|=0x80;
            }

            if (mapas.mons[i].spawn)
            {
                stats|=0x40;
            }

            coords[cnt] = stats;
            cnt++;

            if (mapas.mons[i].spawn)
            {
                int hp = mapas.mons[i].getHP();
                memcpy(&coords[cnt], &hp, sizeof(int));
                cnt+=sizeof(int);
            }

        }
    }

    serveris.sendData(clientindex, coords, cnt);
}
//-----------------------------------
void Game::GetCharData(const unsigned char* bufer, int bufersize, int* index )
{
    if ((bufersize-(*index))>=14)
    {

        unsigned char monum=0;
        memcpy(&monum, &bufer[*index], sizeof(unsigned char));
        *index += sizeof(unsigned char);
        memcpy(&mapas.mons[monum].x,&bufer[*index],sizeof(float));
        *index+=sizeof(float);
        memcpy(&mapas.mons[monum].y,&bufer[*index],sizeof(float));
        *index+=sizeof(float);
        memcpy(&mapas.mons[monum].angle,&bufer[*index],sizeof(float));
        *index+=sizeof(float);
        unsigned char theFrame = 0;
        memcpy(&theFrame, &bufer[*index], sizeof(unsigned char));
        mapas.mons[monum].setFrame(theFrame);
        *index+=sizeof(unsigned char);
        unsigned char stats=bufer[*index];
        if (stats & 0x80)
            mapas.mons[monum].shot=true;
        else
            mapas.mons[monum].shot=false;

        if (stats & 0x40)
        {
            mapas.mons[monum].spawn=true;
        }
        else
        {
            mapas.mons[monum].spawn=false;
        }

        (*index)++;

        if (mapas.mons[monum].spawn)
        {
            int hp = 0;
            memcpy(&hp, &bufer[*index], sizeof(int));
            mapas.mons[monum].setHP(hp);
            *index+=sizeof(int);
        }


    }
}
//---------------------------------------------
void Game::GetMapInfo(const unsigned char* bufer, int* index)
{

    ++(*index);

    int mapnamelen = 0;
    char mapname[255];

    memcpy(&mapnamelen, &bufer[*index], sizeof(int));
    *index += sizeof(int);

    if (mapnamelen)
    {

        memcpy(mapname, &bufer[*index], mapnamelen);
        *index += mapnamelen;
        mapname[mapnamelen] = 0;
    }

    int oldClientCount = otherClientCount;

    memcpy(&otherClientCount, &bufer[*index], sizeof(int));
    *index += sizeof(unsigned int);


    clientIds.destroy();

    for (int i = 0; i < otherClientCount; ++i)
    {
        int id = 0;
        memcpy(&id, &bufer[*index], sizeof(int));
        clientIds.add(id);
        *index += sizeof(int);
    }

    memcpy(&clientMyIndex, &bufer[*index], sizeof(int));
    *index += sizeof(int);

    netGameState = (MultiplayerModes)bufer[*index];
     ++(*index);

     if (mapnamelen)
     {

         if (strcmp(mapname, mapas.name) != 0)
         {  //  Let's load if the map is different
             LoadTheMap(mapname, false, otherClientCount, ENTITY_INITIAL_HP);
             Client_GotMapData = true;
             state = GAMESTATE_GAME;
         }
         else
         {
             populateClientDudes(oldClientCount);
         }
     }
     else
     {
         state = GAMESTATE_GAME;

         if (Client_GotMapData)
         {
            populateClientDudes(oldClientCount);
         }

     }
}
//------------------------------------
void Game::GetServerResurrectMsg(const unsigned char* buffer, int* index)
{
    ++(*index);

    int playerIdx = 0;
    memcpy(&playerIdx, &buffer[*index], sizeof(unsigned));
    *index += sizeof(unsigned);

    printf("MUST RESURRECT monster: %d\n", playerIdx - mapas.enemyCount);

    Dude* player = mapas.getPlayer(playerIdx - mapas.enemyCount);

    player->shot = false;
}

//------------------------------------
void Game::populateClientDudes(int oldClientCount)
{

    for (int i = 0; i < otherClientCount - oldClientCount; i++)
    {
        Dude n;
        n.race = MONSTER_RACE_PLAYER;
        mapas.mons.add(n);
        mapas.mons[mapas.mons.count()-1].appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
    }

    for (int i = 0; i < otherClientCount; ++i)
    {
        mapas.mons[mapas.enemyCount + 1 + i].id = clientIds[i];
    }
}


//------------------------------------
void Game::GetMapData(const unsigned char* bufer, int* index)
{

    int mapHeight = 0;
    int mapWidth = 0;

    memcpy(&mapHeight, &bufer[*index], sizeof(int));
    *index += sizeof(int);
    memcpy(&mapWidth, &bufer[*index], sizeof(int));
    *index += sizeof(int);

    if (mapHeight && mapWidth)
    {
        mapas.destroy();
        mapas.tiles = new unsigned char * [mapHeight];

        for (int i = 0; i < mapHeight; ++i)
        {
            mapas.tiles[i] = new unsigned char[mapWidth];

            for (int a = 0; a < mapWidth; ++a)
            {

                memcpy(&mapas.tiles[i][a], &bufer[*index], sizeof(unsigned char));
                *index += sizeof(unsigned char);
            }
        }

        mapas._height = mapHeight;
        mapas._width = mapWidth;

        mapas.buildCollisionmap();

    }

    int missionItems = 0;
    memcpy(&missionItems, &bufer[*index], sizeof(int));
    *index += sizeof(int);
    mapas.misionItems = missionItems;


    int moncount = 0;
    memcpy(&moncount, &bufer[*index], sizeof(int));
    *index += sizeof(int);

    if (!mapas.mons.count()) //  No monsters in the map ? Must be coop map
    {
        mapas.enemyCount = moncount;

        for (int i = 0; i < moncount; i++)
        {
            Dude monster;
            monster.id = i;
            monster.initMonsterHP();
            mapas.addMonster(monster);
        }


        Dude d;
        d.id = mapas.enemyCount;
        d.race = MONSTER_RACE_PLAYER;
        d.setWeaponCount(PLAYER_SIMULTANEOUS_WEAPONS);
        d.setSkinCount(PLAYER_MAX_SKIN_COUNT);
        mapas.addMonster(d);
        mapas.mons[mapas.mons.count()-1].appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());

        for (int i = 0; i < otherClientCount; i++)
        {
            Dude n;
            n.setWeaponCount(PLAYER_SIMULTANEOUS_WEAPONS);
            n.setSkinCount(PLAYER_MAX_SKIN_COUNT);
            n.setFrame((n.activeSkin[n.getCurrentWeapon()] + 1) * 4 - 2);

            n.id = clientIds[i];
            n.race = MONSTER_RACE_PLAYER;
            mapas.mons.add(n);
            mapas.mons[mapas.mons.count() - 1].appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
        }

        AdaptMapView();

        Client_GotMapData = true;

    }

    for (int i = 0; i < moncount; i++)
    {
        memcpy(&mapas.mons[i].race, &bufer[*index], sizeof(int));
        *index+=sizeof(int);
    }

    int itmcount = 0;
    memcpy(&itmcount, &bufer[*index], sizeof(int));
    *index+=sizeof(int);

    for (int i = 0; i < itmcount; i++)
    {
        Item itm;

        memcpy(&itm.x, &bufer[*index], sizeof(float));
        *index += sizeof(float);
        memcpy(&itm.y, &bufer[*index], sizeof(float));
        *index += sizeof(float);
        memcpy(&itm.value, &bufer[*index], sizeof(int));
        *index += sizeof(int);
        mapas.addItem(itm.x, itm.y, itm.value);
    }

}
//------------------------------------
void Game::ServerParseClientGameEquip(const unsigned char* buffer, unsigned* bufferindex, int clientIndex)
{
    ++(*bufferindex);

    if (clientIndex == -1)
    {
        *bufferindex += sizeof(unsigned);
        return;
    }

    unsigned cartIdx = 0;
    memcpy(&cartIdx, &buffer[*bufferindex], sizeof(unsigned));
    *bufferindex += sizeof(unsigned);


    const int entityIdx = mapas.enemyCount + 1 + clientIndex;

    int backup = 0;
    if (mapas.mons[entityIdx].equipedGame)
    {
        backup = mapas.mons[entityIdx].equipedGame;
    }

    const int game = clientLoot[clientIndex].cartridges[cartIdx];

    equipCartridge(&mapas.mons[entityIdx], game);
    clientLoot[clientIndex].cartridges.remove(cartIdx);

    if (backup)
    {
        clientLoot[clientIndex].cartridges.add(backup);
    }

    SendServerEquipedCartridgeToAllClients(entityIdx, game);
}
//----------------------------------
void Game::ServerParseCharacterData(const unsigned char* bufer, unsigned * buferindex, int clientIndex)
{
    ++(*buferindex);

    if (clientIndex == -1) // client not found
    {
        *buferindex += sizeof(float);
        *buferindex += sizeof(float);
        *buferindex += sizeof(float);
        (*buferindex)++;
        return;
    }

    const unsigned clientIdx = mapas.enemyCount + 1 + clientIndex;

    memcpy(&mapas.mons[clientIdx].x, &bufer[*buferindex], sizeof(float));
    *buferindex += sizeof(float);
    memcpy(&mapas.mons[clientIdx].y, &bufer[*buferindex], sizeof(float));
    *buferindex += sizeof(float);
    memcpy(&mapas.mons[clientIdx].angle, &bufer[*buferindex], sizeof(float));
    *buferindex += sizeof(float);
    unsigned char theFrame = 0;
    memcpy(&theFrame, &bufer[*buferindex], sizeof(unsigned char));
    mapas.mons[clientIdx].setFrame(theFrame);
    (*buferindex)++;

    unsigned char stats = bufer[*buferindex];
    //mapas.mons[clientIdx].shot = (stats & 0x80) ? true : false;
    mapas.mons[clientIdx].spawn = (stats & 0x40) ? true : false;
}
//--------------------------------------------------------------
void Game::ServerParseWeaponShot(const unsigned char* buffer, unsigned * bufferindex, int clientIndex)
{
    ++(*bufferindex);
    unsigned char weaponType = 0;


    if (clientIndex != -1) //only if client exists
    {
        memcpy(&mapas.mons[mapas.enemyCount + 1 + clientIndex].ammo, &buffer[*bufferindex], sizeof(int));
    }

    *bufferindex += sizeof(int);

    memcpy(&weaponType, &buffer[*bufferindex], sizeof(unsigned char));

    *bufferindex += sizeof(unsigned char);


    if (clientIndex != -1) // only if client exists
    {

        mapas.mons[mapas.enemyCount + 1 + clientIndex].shoot(true, (WeaponTypes)weaponType, &bulbox);


        for (unsigned int a = 0; a < serveris.clientCount(); a++)
        {
            if (a != (unsigned)clientIndex)
            {
                SendBulletImpulse(mapas.enemyCount + 1 + clientIndex, mapas.mons[mapas.enemyCount + 1 + clientIndex].ammo, a, weaponType);
            }
        }
    }

}

//--------------------------------------------------------------
void Game::ServerParseClientResurrect(unsigned* bufferindex, int clientIndex)
{
    ++(*bufferindex);

    Dude* thatClient = mapas.getPlayer(clientIndex + 1);
    Vector3D dir = MakeVector(PLAYER_RADIUS, 0.f, thatClient->angle);

    Vector3D ic(thatClient->x + dir.x, thatClient->y + dir.x, 0);

    for (unsigned i = mapas.enemyCount; i < mapas.mons.count(); ++i)
    {

        if (i != (unsigned)mapas.enemyCount + clientIndex + 1)
        {
            if (mapas.mons[i].shot)
            {

                if (CirclesColide(mapas.mons[i].x, mapas.mons[i].y, 10.f, ic.x, ic.y, 10.f))
                {
                    mapas.mons[i].shot = false;
                    mapas.mons[i].setFrame(mapas.mons[i].activeSkin[mapas.mons[i].getCurrentWeapon()] * 4);
                    mapas.mons[i].heal();
                    //  ok now let's send this to all clients
                    for (unsigned a = 0; a < serveris.clientCount(); ++a)
                    {
                        if (a != (unsigned)clientIndex)
                        {
                            SendResurrectMessageToClient(a, i);
                        }
                    }
                }
            }
        }
    }

}
//--------------------------------------------------------------
void Game::GetDoorInfo(const unsigned char* bufer, unsigned * index, int* dx, int* dy, unsigned char* frame)
{

    int doorx,doory;
    unsigned char doorframe=0;
    memcpy(&doorx,&bufer[*index],sizeof(int));
    *index+=sizeof(int);
    memcpy(&doory,&bufer[*index],sizeof(int));
    *index+=sizeof(int);
    memcpy(&doorframe,&bufer[*index],sizeof(unsigned char));
    *index+=sizeof(unsigned char);

    mapas.tiles[doory][doorx]=doorframe;
    if ((doorframe==65) || (doorframe==67) || (doorframe==69) || (doorframe==71))
    {
        mapas._colide[doory][doorx] = true;
    }
    else
    {
        mapas._colide[doory][doorx] = false;
    }

    if (dx)
    {
        *dx=doorx;
    }

    if (dy)
    {
        *dy = doory;
    }
    if (frame)
    {
        *frame = doorframe;
    }
}
//---------------------------------------
void Game::GetNewItemInfo(unsigned char* bufer, int* index)
{
    float x,y;
    int value;
    memcpy(&x,&bufer[*index],sizeof(float));
    *index+=sizeof(float);
    memcpy(&y,&bufer[*index],sizeof(float));
    *index+=sizeof(float);
    memcpy(&value,&bufer[*index],sizeof(int));
    *index+=sizeof(int);

    mapas.addItem(x,y,value);
}
//---------------------------------------
void Game::GetClientAtackImpulse(const unsigned char* buf, unsigned * index, int ClientIndex)
{
    ++(*index);
    int victim;
    int hp;
    memcpy(&victim, &buf[*index], sizeof(int));
    *index += sizeof(int);
    memcpy(&hp, &buf[*index], sizeof(int));
    *index += sizeof(int);


    mapas.mons[victim].hit = true;
    mapas.mons[victim].setHP(hp);
    mapas.mons[victim].lastDamagedBy = mapas.mons[mapas.enemyCount + 1 + ClientIndex].id;


    for (unsigned clientIdx = 0; clientIdx < serveris.clientCount(); ++clientIdx)
    {

        if (ClientIndex != (int)clientIdx) // don't send to the same client who did the attack
        {
            SendServerMeleeImpulseToClient(clientIdx, victim, hp, mapas.mons[victim].lastDamagedBy);
        }
    }

}
//----------------------------------
void Game::GetAttackImpulseFromServer(const unsigned char* buf, int* index)
{
    ++(*index);

    int victim = 0;
    int hp = 0;
    int attackerId = 0;

    memcpy(&victim, &buf[*index], sizeof(int));
    *index += sizeof(int);
    memcpy(&hp, &buf[*index], sizeof(int));
    *index += sizeof(int);
    memcpy(&attackerId, &buf[*index], sizeof(int));
    *index += sizeof(int);

    mapas.mons[victim].hit = true;
    mapas.mons[victim].setHP(hp);
    mapas.mons[victim].lastDamagedBy = attackerId; //who's the attacker ?

}
//---------------------------------

void Game::ParseMessagesServerGot()
{

    if (!serveris.storedPacketCount())
    {
        return;
    }


    //printf("client packets to parse %u\n", serveris.storedPacketCount());

    for (int msgIdx = serveris.storedPacketCount() - 1; msgIdx >= 0; --msgIdx)
    {

        Message* msg = serveris.fetchPacket(msgIdx);

        if (msg->parsed)
        {
            continue;
        }


        int clientIdx = serveris.findClientByAddress(msg->senderAddress);

        unsigned index = 0;

        while (index < msg->length)
        {

            char NetworkHeader[4] = {0};

            if (msg->length - index >= NET_HEADER_LEN)
            {
                memcpy(&NetworkHeader, &(msg->data)[index], NET_HEADER_LEN);
                index += NET_HEADER_LEN;
                if (strcmp(NetworkHeader, NET_HEADER) != 0)
                {
                    msg->parsed = true;
                    continue;
                }
            }

            NetworkCommands command = NET_NOP;

            if (msg->length - index >= 1)
            {
                memcpy(&command, &(msg->data)[index], 1);
            }

            switch(command)
            {

                case NET_CLIENT_MSG_CONNECT:
                    {
                        printf("CLIENT CONNECTED!\n");
                        ++index;

                        Dude newclient;
                        newclient.race = MONSTER_RACE_PLAYER;
                        mapas.mons.add(newclient);
                        mapas.mons[mapas.mons.count() - 1].appearInRandomPlace(mapas._colide, mapas.width(), mapas.height());
                        int zeroFrags = 0;
                        fragTable.add(zeroFrags);

                        ClientLoot lt;
                        clientLoot.add(lt);


                        ClientFootprint fp;
                        fp.address = msg->senderAddress;
                        serveris.addClient(fp);

                        mapas.mons[mapas.mons.count()-1].id = mapas.mons[mapas.mons.count()-2].id + 1;

                        for (int i =0; i < (int)serveris.clientCount(); i++)
                        {
                            SendMapInfo(i, mapas);
                        }

                        SendMapData(serveris.clientCount() - 1, mapas);
                        ServerSendTimerSync(serveris.clientCount() - 1);

                    } break;

                case NET_CLIENT_MSG_GAME_EQUIPED   : ServerParseClientGameEquip(msg->data, &index, clientIdx); break;
                case NET_CLIENT_MSG_CHARACTER_DATA : ServerParseCharacterData(msg->data, &index, clientIdx);   break;
                case NET_CLIENT_MSG_WEAPON_SHOT    : ServerParseWeaponShot(msg->data, &index, clientIdx);      break;
                case NET_CLIENT_MSG_COOP_RESURRECT : ServerParseClientResurrect(&index, clientIdx);            break;
                case NET_CLIENT_MSG_ITEM           :
                    {
                        ++index;
                        int itmindex = 0;
                        memcpy(&itmindex, &(msg->data)[index], sizeof(int));
                        index+=sizeof(int);

                        if (mapas.items[itmindex].value > ITEM_MEDKIT)
                        {
                            clientLoot[clientIdx].cartridges.add(mapas.items[itmindex].value);
                        }

                        mapas.removeItem(itmindex);

                        for (unsigned int a = 0; a < serveris.clientCount(); a++)
                        {
                            if (a != (unsigned)clientIdx)
                            {
                                SendItemSRemove(itmindex,a,true);
                            }
                        }

                    } break;

                case NET_CLIENT_MSG_DOOR:
                    {
                        ++index;
                        int dx,dy;
                        unsigned char frame;
                        GetDoorInfo(msg->data, &index, &dx,&dy,&frame);

                        for (unsigned int a=0; a < serveris.clientCount(); a++)
                        {
                            if (a != (unsigned)clientIdx)
                            {
                                SendServerDoorState(a,dx,dy,frame);
                            }
                        }
                    } break;

                case NET_CLIENT_MSG_NEXT_LEVEL:
                    {
                        ++index;
                        mapai.current++;
                        const int otherPlayerCount = serveris.clientCount();

                        Dude* player = mapas.getPlayer();
                        GoToLevel(player->getHP(), player->ammo, mapai.current, otherPlayerCount);

                        for (int a = 0; a < (int)serveris.clientCount(); ++a)
                        {
                            SendMapInfo(a, mapas);
                            SendMapData(a, mapas);
                        }


                    } break;

                case NET_CLIENT_MSG_MELEE_ATTACK : GetClientAtackImpulse(msg->data, &index, clientIdx); break;

                case NET_CLIENT_MSG_QUIT:
                    {
                        ++index;

                        for (unsigned i = 0; i < serveris.clientCount(); ++i)
                        {
                            if (i != (unsigned)clientIdx)
                            {
                                char data[MAX_MESSAGE_DATA_SIZE];
                                int len = 0;

                                strcpy(data, NET_HEADER);
                                len += NET_HEADER_LEN;

                                data[len] = NET_SERVER_MSG_REMOVE_CHARACTER;
                                ++len;
                                int idx = mapas.enemyCount + 1 + clientIdx;
                                memcpy(&data[len], &idx, sizeof(int));
                                len += sizeof(int);

                                int newIndexForThatClient = (i > (unsigned)clientIdx) ? i - 1 : i;

                                memcpy(&data[len], &newIndexForThatClient, sizeof(int));
                                len += sizeof(int);

                                serveris.sendData(i, data, len);
                            }
                        }

                        serveris.removeClient(clientIdx);
                        clientLoot.remove(clientIdx);
                        mapas.mons.remove(mapas.enemyCount + clientIdx + 1);
                    } break;

                case NET_CLIENT_MSG_PONG:
                    {
                        ++index;

                    } break;
                default:
                    {
                        ++index;
                    }
            }
        }

        msg->parsed = true;

    }

    for (int i = serveris.storedPacketCount() - 1; i >= 0 ; --i)
    {
        Message* msg = serveris.fetchPacket(i);
        if (msg->parsed)
        {
            serveris.discardPacket(i);
        }
    }

    //printf("packets after %u\n", serveris.storedPacketCount());


}
//-----------------------------
void Game::ServerSendTimerSync(unsigned clientIdx)
{
    char buffer[MAX_MESSAGE_DATA_SIZE];
    int index = 0;
    memcpy(&buffer[index], NET_HEADER, NET_HEADER_LEN);
    index += NET_HEADER_LEN;
    buffer[index] = NET_SERVER_MSG_SYNC_TIMER;
    ++index;
    memcpy(&buffer[index], &timeleft, sizeof(int));
    index += sizeof(int);

    serveris.sendData(clientIdx, buffer, index);
}
//------------------------------
void Game::GetServerTimeMsg(const unsigned char* buffer, int* bufferindex)
{
    ++(*bufferindex);
    memcpy(&timeleft, &buffer[*bufferindex], sizeof(int));
    *bufferindex += sizeof(int);
}
//------------------------------
void Game::GetServerCoopGameOverMsg(int* bufferindex)
{
    ++(*bufferindex);
    gameOver = true;
}
//-----------------------------
void Game::equipCartridge(Dude* dude, int game)
{
    dude->equipedGame = game;

    if (dude->equipedGame == ITEM_GAME_FART_NIGHT)
    {
        dude->setupToxicParticles();
    }
    else
    {
        dude->stopParticles();
    }

    for (int i = 0; i < 2; ++i)
    {
        dude->activeSkin[i] = gameData.getGame(dude->equipedGame - ITEM_GAME_NINJA_MAN)->skins[i];
    }


    dude->setFrame(dude->activeSkin[dude->getCurrentWeapon()] * 4);
}


//------------------------------
void Game::GetServerEquipedGame(const unsigned char* buffer, int * bufferindex)
{
    ++(*bufferindex);
    unsigned idx = 0;
    memcpy(&idx, &buffer[*bufferindex], sizeof(unsigned));
    *bufferindex += sizeof(unsigned);
    int game = 0;
    memcpy(&game, &buffer[*bufferindex], sizeof(int));
    *bufferindex += sizeof(int);

    assert(idx < mapas.mons.count());

    Dude* dude = &mapas.mons[idx];
    equipCartridge(dude, game);
}
//------------------------------
void Game::ParseMessagesClientGot()
{
    if (!client.storedPacketCount())
    {
        return;
    }


    //printf("server packets to parse %u\n", client.storedPacketCount());

    for (unsigned msgIdx = 0; msgIdx < client.storedPacketCount(); ++msgIdx)
    {

        Message* msg = client.fetchPacket(msgIdx);

        if (msg->parsed)
        {
            continue;
        }


        int index = 0;

        while ((unsigned)index < msg->length)
        {
            NetworkCommands command = NET_NOP;
            char NetworkHeader[NET_HEADER_LEN + 1] = {0};

            if (msg->length - index >= NET_HEADER_LEN)
            {
                memcpy(&NetworkHeader, &(msg->data)[index], NET_HEADER_LEN);
                index += NET_HEADER_LEN;
                if (strcmp(NetworkHeader, NET_HEADER) != 0)
                {
                    msg->parsed = true;
                    continue;
                }
            }

            if (msg->length - index >= 1)
            {
                memcpy(&command, &(msg->data)[index], 1);
            }

            switch(command)
            {

                case NET_SERVER_MSG_CHARACTER_DATA:
                    {
                        ++index;
                        GetCharData(msg->data, msg->length, &index);
                    } break;

                case NET_SERVER_MSG_SHUTTING_DOWN:
                    {
                        client.shutdown();
                        netMode = NETMODE_NONE;
                        goToEnding();
                        doFadein = true;
                        fadeTimer = 0;
                        objectivetim = 200;
                        return;
                    } break;

                case NET_SERVER_MSG_GAME_EQUIPED  :  GetServerEquipedGame(msg->data, &index);  break;
                case NET_SERVER_MSG_COOP_RESURRECT:  GetServerResurrectMsg(msg->data, &index); break;
                case NET_SERVER_MSG_SERVER_INFO   :  GetMapInfo(msg->data, &index);            break;
                case NET_SERVER_MSG_SYNC_TIMER    :  GetServerTimeMsg(msg->data, &index);      break;
                case NET_SERVER_MSG_COOP_GAME_OVER:  GetServerCoopGameOverMsg(&index);         break;

                case NET_SERVER_MSG_MAP_DATA:
                    {
                        ++index;
                        GetMapData(msg->data, &index);
                    } break;

                case NET_SERVER_MSG_REMOVE_CHARACTER:
                    {
                        ++index;
                        int idxToRemove;
                        memcpy(&idxToRemove, &(msg->data)[index], sizeof(int));
                        index += sizeof(int);

                        memcpy(&clientMyIndex, &(msg->data)[index], sizeof(int));
                        index += sizeof(int);
                        mapas.removeMonster(idxToRemove);

                    } break;

                case NET_SERVER_MSG_KILL_CHARACTER:
                    {
                        ++index;
                        int victimIdx = -1;
                        memcpy(&victimIdx, &(msg->data)[index], sizeof(int));
                        index += sizeof(int);

                        if (victimIdx != -1)
                        {
                            KillEnemy(victimIdx);
                        }
                    } break;

                case NET_SERVER_MSG_FRAG:
                    {
                        ++index;
                        memcpy(&frags, &(msg->data)[index], sizeof(int));
                        index += sizeof(int);
                    } break;

                case NET_SERVER_MSG_WEAPON_SHOT:
                    {
                        ++index;
                        int ind = 0;
                        memcpy(&ind, &(msg->data)[index], sizeof(int));
                        index += sizeof(int);

                        memcpy(&mapas.mons[ind].ammo, &(msg->data)[index],sizeof(int));
                        index+=sizeof(int);
                        unsigned char weaponType = 0;
                        memcpy(&weaponType, &(msg->data)[index], sizeof(unsigned char));
                        index += sizeof(unsigned char);

                        mapas.mons[ind].shoot(true, (WeaponTypes)weaponType, &bulbox);
                    } break;

                case NET_SERVER_MSG_ITEM:
                    {
                        ++index;
                        int itmindex=0;
                        memcpy(&itmindex, &(msg->data)[index], sizeof(int));
                        index+=sizeof(int);
                        unsigned char isPlayerTaked = 0;
                        memcpy(&isPlayerTaked, &(msg->data)[index], sizeof(unsigned char));
                        index++;

                        mapas.removeItem(itmindex);

                    } break;

                case NET_SERVER_MSG_DOOR:
                    {
                        ++index;
                        GetDoorInfo(msg->data, (unsigned*)&index, 0,0,0);
                    } break;

                case NET_SERVER_MSG_NEW_ITEM:
                    {
                        ++index;
                        GetNewItemInfo(msg->data, &index);
                    } break;

                case NET_SERVER_MSG_MELEE_ATTACK: GetAttackImpulseFromServer(msg->data, &index); break;
                case NET_SERVER_MSG_PING:
                    {
                        ++index;
                        char buf[MAX_MESSAGE_DATA_SIZE];
                        strcpy(buf, NET_HEADER);
                        buf[NET_HEADER_LEN] = NET_CLIENT_MSG_PONG;
                        client.sendData(buf, NET_HEADER_LEN + 1);

                    } break;

                default:
                    {
                        index++;
                    }
            }

        } //while

        msg->parsed = true;

    } //for
    
    for (int i = client.storedPacketCount() - 1; i >= 0 ; --i)
    {
        Message* msg = client.fetchPacket(i);
        if (msg->parsed)
        {
            client.discardPacket(i);
        }
    }

    //printf("packets after %u\n", client.storedPacketCount());



}


//---------------------------------------
void Game::LoadShader(ShaderProgram* shader, const char* name, bool useVulkan, bool useUVS)
{
    shader->create(useVulkan);

    char error[1024];
    char buf[512];

    if (!useVulkan)
    {
        Shader vert;
        Shader frag;

        printf("Loading vertex shader...\n");
        sprintf(buf, "shaders/%s.vert", name);

#ifdef __ANDROID__
        vert.loadGL(VERTEX_SHADER, buf, AssetManager);
#else
        vert.loadGL(VERTEX_SHADER, buf);
#endif

        printf("Loading fragment shader...\n");
        sprintf(buf, "shaders/%s.frag", name);
#ifdef __ANDROID__
        frag.loadGL(FRAGMENT_SHADER, buf, AssetManager);
#else
        frag.loadGL(FRAGMENT_SHADER, buf);
#endif

        shader->attach(vert);
        shader->attach(frag);
        shader->link();

        shader->getLog(error, 1024);
        if (strlen(error)) 
        {
#ifdef __ANDROID__
            LOGI("--%s--", buf);
            LOGI("%s", error);
        }
        LOGI("---------------");
#else
            printf("--%s--\n", buf);
            puts(error);
        }
        puts("-----------");
#endif
    }
    else //VULKAN
    {
        Shader vert;
        Shader frag;

        sprintf(buf, "shaders/%s_vert.spv", name);
        vert.loadVK(VERTEX_SHADER, buf, vulkanDevice);

        shader->attach(vert);

        sprintf(buf, "shaders/%s_frag.spv", name);
        frag.loadVK(FRAGMENT_SHADER, buf, vulkanDevice);

        shader->attach(frag);
        shader->buildVkPipeline(vulkanDevice, vkPhysicalDevice, vkRenderPass, sys, useUVS);
    }

}

//----------------------
void Game::loadConfig()
{
    char buf[1024];
    printf("Document path: %s\n", DocumentPath);
    sprintf(buf, "%s/settings.cfg", DocumentPath);
    sys.load(buf);

    ScreenWidth = sys.ScreenWidth * sys.screenScaleX;
    ScreenHeight = sys.ScreenHeight * sys.screenScaleY;
    windowed = sys.useWindowed;
    renderer = sys.renderIdx;

    sys.write(buf);
}


//-----------------------------------------
void Game::init(bool useVulkan)
{

   srand(time(0));


   if (!useVulkan)
   {
#ifndef __ANDROID__
        LoadExtensions();
#endif
   }
        printf("Creating shaders...\n");

        LoadShader(&defaultShader, "default", useVulkan, true);
        LoadShader(&colorShader, "justcolor", useVulkan, false);


    if (!useVulkan)
    {
        colorShader.use(vkCmd);
        glEnable(GL_TEXTURE_2D);
        glDepthFunc(GL_LEQUAL);

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }
   else  //  Vulkan
   {
   }


    MatrixOrtho(0.0, ScreenWidth / (float)sys.screenScaleX, ScreenHeight / (float)sys.screenScaleY, 0.0, -400, 400, OrthoMatrix);

    pics.load("pics/imagesToLoad.xml", useVulkan, vulkanDevice, vkPhysicalDevice, vkCommandPool, vkGraphicsQueue);


    Smenu menu;
    strcpy(menu.opt[0], "Single Player");
    strcpy(menu.opt[1], "Network Game");
    strcpy(menu.opt[2], "Collection");
    strcpy(menu.opt[3], "Options");
    strcpy(menu.opt[4], "Exit");
    menu.count = 5;
    mainmenu.init(0, sys.ScreenHeight - 150, "", menu, 0);
    mainmenu.activate();
    strcpy(menu.opt[0],"Start server");
    strcpy(menu.opt[1],"Join server");
    menu.count=2;
    netmenu.init(0,sys.ScreenHeight-100,"Network Game:",menu,0);

    strcpy(menu.opt[0],"Coop");
    strcpy(menu.opt[1],"DeathMatch");
    menu.count=2;
    netgame.init(0,sys.ScreenHeight-100,"Game Type:",menu,0);

    for (int i = 0; i < mapai.count(); ++i)
    {
        mapai.getMapName(i, menu.opt[i]);
    }

    menu.count = mapai.count();
    mapmenu.init(0, sys.ScreenHeight - mapai.count() * 20 - 32, "Select map:", menu, 0);

    strcpy(menu.opt[0],"Music Volume");
    strcpy(menu.opt[1],"Sound fx Volume");
    menu.count = 2;
    options.init(0,sys.ScreenHeight-100,"Options:",menu,0);

    ipedit.init(0,sys.ScreenHeight-100,"Enter Server's IP",20);

    SfxVolumeC.init(20,sys.ScreenHeight-100,"Sfx Volume:",0,10000,100);
    MusicVolumeC.init(20, sys.ScreenHeight-100, "Music Volume:", (long)(sys.musicVolume * 1000), 1000, 10);


    intro.load("data/intro.itf");

    InitAudio();
    ResetVolume();

    gameData.load("data/gameData.xml");

    SaveGame::load(DocumentPath, &stash);
    cartridgeCollection.init(&stash);

    PlayNewSong("evil.ogg");

    Works = true;
}
//--------------------------------
void Game::destroy()
{
    switch(netMode)
    {
        case NETMODE_NONE: break;
        case NETMODE_SERVER: StopServer(); break;
        case NETMODE_CLIENT: QuitServer(); break;
    }

    SaveGame::save(DocumentPath, &stash);
    music.release();
    SoundSystem::getInstance()->exit();

    mapas.destroy();
    mapai.Destroy();
    pics.destroy();

    bulbox.destroy();
}
