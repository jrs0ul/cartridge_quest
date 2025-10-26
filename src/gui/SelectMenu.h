#ifndef	_SELECTMENIU_H_
#define _SELECTMENIU_H_

#include <cstring>
#include "BasicUiControl.h"
#include "../SpriteBatcher.h"

struct Smenu{
  char opt[20][30];
  unsigned char pics[20];
  unsigned char ids[20];
  unsigned char count;
  Smenu(){memset(pics,0,20); memset(ids,0,20);}
 };

//--------------------------------------



class SelectMenu : public BasicControl
{
    public:
        char title[30];
        Smenu selection;
        unsigned pressedkey;
        unsigned int width;
        unsigned int height;
        unsigned char defstate;
        unsigned char state;

        bool selected;
        bool canceled;

        SelectMenu(){selected = false; canceled = false;}

        void init(unsigned int dx, unsigned int dy, const char* dt, Smenu& dsel, unsigned char dst,unsigned int dheight = 0);
        // resets the selection
        void reset();
        //changes the state depending on input keys
        void getInput(const unsigned char* keys, const unsigned char* oldKeys);
        void draw(SpriteBatcher& pics, unsigned rod,  unsigned font, unsigned icons = 0);

};

#endif
