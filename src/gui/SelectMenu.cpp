#include "Text.h"
#include "SelectMenu.h"





void SelectMenu::init(unsigned int dx, unsigned int dy, const char* dt, Smenu& dsel, unsigned char dst,unsigned int dheight)
{
    setpos(dx, dy);
    strcpy(title,dt);

    for (int i=0; i < dsel.count; ++i)
    {
        strcpy(selection.opt[i],dsel.opt[i]);
    }

    selection.count=dsel.count;

    if ((strlen(title)==0) || (strlen(selection.opt[0])>strlen(title)))
    {
        width=(int)strlen(selection.opt[0])*11+70;
    }
    else
    {
        width=(int)strlen(title)*11+70;
    }

    memcpy(selection.pics,dsel.pics,20);

    if (dheight == 0)
    {
        height = selection.count*20+50;
    }
    else
    {
        height = dheight;
    }

    defstate = dst;
    state = defstate;
    selected = false; //dar nieko neisirinkom
    canceled = false;
    deactivate();
    pressedkey = 0;
}
//--------------------------------------------
void SelectMenu::reset()
{
    state = defstate;
    selected = false;
    canceled = false;
}
//--------------------------------------------
void SelectMenu::getInput(const unsigned char* keys, const unsigned char* oldKeys)
{

    if (keys[4] && !oldKeys[4]) 
    {
        if (selection.count)
        {
            selected = true;      //enter
        }
    }

    if (keys[5] && !oldKeys[5]) 
    {
        canceled=true;      //esc
    }

    if (selection.count)
    {

        if (keys[0] && !oldKeys[0])
        {
            if (state>0) //up{
                state--;
            else
                state = selection.count - 1;
        }
        else if (keys[1] && !oldKeys[1])
        {

            if (state < selection.count - 1)  //down
                state++;
            else
                state = 0;
        }


        }

    }
    //--------------------------------------------
    void SelectMenu::draw(SpriteBatcher& pics, unsigned rod,  unsigned font, unsigned icons)
    {

        unsigned newcount=0;
        unsigned start=0;
        int half = (((height-28)/20)/2);
        int tmpheight = height;

        if (selection.count * 20 + 28 > tmpheight)
        {
            if (((tmpheight-28)/20)+state/half>selection.count)
            {
                newcount=selection.count;
            }
            else
            {
                newcount=((height-28)/20)+state/half;
            }

            start=state/half;
        }
        else
        {
            newcount=selection.count;
        }

        if ((icons) && (((newcount - start) * pics.getInfo(icons)->theight) + 28 > (unsigned)tmpheight))
        {
            height=(newcount-start) * pics.getInfo(icons)->theight + 40;
        }

        //DrawBlock(device,spraitas,x,y,width,height,0,0,200);
        WriteText(getX() + 12, getY() + 2, pics, font, title, 1.0f, 1.0f, COLOR(0,0,0, 1.f), COLOR(0,0,0,1.f)); 
        WriteText(getX() + 10, getY() + 4, pics, font, title); 


        for (unsigned i = start; i< newcount; i++)
        {
            if (icons)
            {
                pics.draw(icons, getX() + 16, getY() + 28 + ((i - start) * pics.getInfo(icons)->theight), selection.pics[i]);
                WriteText(getX() + 20 + pics.getInfo(icons)->theight,
                        getY() + 28 + ((i-start) * pics.getInfo(icons)->theight),
                        pics, font, selection.opt[i], 1.0f,1.2f, COLOR(0,0,0, 1.f));

                WriteText(getX() + 20 + pics.getInfo(icons)->theight,
                        getY() + 28 + ((i-start) * pics.getInfo(icons)->theight),
                        pics, font, selection.opt[i]);

            }
            else
            {
                WriteText(getX() + 34, getY() + 26+((i-start)*20), pics, font, selection.opt[i], 1.0f,1.0f, COLOR(0,0,0,1.f), COLOR(0,0,0,1.f));
                WriteText(getX() + 32, getY() + 28+((i-start)*20), pics,font,selection.opt[i]);

            }
        }


        if (start>0)
        {
            pics.draw(rod, getX() + width-18, getY() + 3, 1);
        }

        if (newcount<selection.count)
        {
            pics.draw(rod, getX() + width-18, getY() + height-19,2);
        }

        //coolframe(x,y,width,height,frm,spraitas);

        int space=20;
        if (icons)
        {
            space = pics.getInfo(icons)->theight;
        }

        if ((rand()%100)%10 == 0)
        {
            pics.draw(rod, getX() + 3, getY() + 28+((state-start)*space));
        }
        else
        {
            pics.draw(rod, getX() + 5, getY() + 28+((state-start)*space));
        }

    }

