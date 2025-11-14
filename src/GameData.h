#pragma once

#include "DArray.h"

class AAssetManager;

struct GameDescription
{
    char name[128];
    char description[256];
    char description1[256];
    char effect[256];
    char effect1[256];
    int skins[2];

    GameDescription() 
    : name("")
    , description("")
    , description1("")
    , effect("")
    , effect1("")
    , skins{0, 1}
    {
    }
};

class GameData
{

    DArray<GameDescription> games;

public:
#ifdef ANDROID
    bool load(const char* path, AAssetManager* assman);
#else
    bool load(const char* path);
#endif
    GameDescription* getGame(unsigned index);
    ~GameData(){games.destroy();}
};
