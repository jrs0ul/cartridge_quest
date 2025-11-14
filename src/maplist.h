#ifndef MAPLIST_H
#define MAPLIST_H

class AAssetManager;

class MapList
{
    int _count;
    char**maps;

public:

    int current;

#ifdef ANDROID
    MapList(AAssetManager* assman);
#else
    MapList();
#endif
    void Destroy();
    int count(){return _count;}
    void getMapName(int i, char* name);
};




#endif //MAPLIST_H
