#include "Collection.h"
#include "SpriteBatcher.h"
#include "Item.h"
#include "ActionKeys.h"


void Collection::init(DArray<int>* lootptr)
{
    loot = lootptr;
}

void Collection::getInput(const unsigned char* keys, const unsigned char* oldKeys)
{
    if (keys[ACTION_BACK] && !oldKeys[ACTION_BACK])
    {
        canceled = true;
    }

}

void Collection::draw(SpriteBatcher& pics)
{
    int posY = 150;

    int idx = 0;

    for (int i = ITEM_GAME_NINJA_MAN; i < ITEM_GAME_MIGHTY_BLAZER + 1; ++i)
    {

        const int posX = idx * 34 + 20;

        pics.draw(-1, posX, posY, 0, false, 32, 32, 0, COLOR(1,1,1,0.5), COLOR(1,1,1,0.5));


        bool found = false;

        for (unsigned a = 0; a < loot->count(); ++a)
        {
            if ((*loot)[a] == i)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            pics.draw(11, posX, posY, i - ITEM_GAME_NINJA_MAN, false);
        }

        ++idx;

        if (idx > 7)
        {
            idx =  0;
            posY += 34;
        }
    }
}
