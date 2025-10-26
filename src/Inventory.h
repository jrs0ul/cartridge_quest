#pragma once

#include "gui/BasicUiControl.h"
#include "DArray.h"

class SpriteBatcher;
class GameData;

class Inventory : public BasicControl
{
    int state;
    bool selected;
    bool canceled;
public:
    Inventory(){state = 0; canceled = false; selected = false;}
    void draw(SpriteBatcher& pics, DArray<int>& loot, GameData& gd);
    void getInput(const unsigned char* keys, const unsigned char* oldKeys, DArray<int>& loot);
    void reset(){canceled = false; selected = false;}
    unsigned getSelected(){return state;}
    bool isSelected(){return selected;}
    bool isCanceled(){return canceled;}
};
