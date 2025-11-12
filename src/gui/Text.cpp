#include "Text.h"
#include "../SpriteBatcher.h"


void WriteText(unsigned int x, unsigned int y,
               SpriteBatcher& pics, int font,
               const char* s,
               float scalex, float scaley,
               COLOR c1, COLOR c2)
{

    for (unsigned int i = 0; i < strlen(s); i++)
    {
        pics.draw(font, x+(11 * scalex)*i, y, ((unsigned char)s[i])-32,
                  false, scalex, scaley, 0.0f, c1, c2);
    }
}
//-----------------------------------------------------
void WriteSmallText(unsigned int x, unsigned int y,
                    SpriteBatcher& pics, int font,
                    const char* s,
                    float scalex, float scaley,
                    COLOR c1, COLOR c2)
{

    for (unsigned int i = 0; i < strlen(s); i++)
    {
        pics.draw(font, x + (6 * scalex)*i, y, ((unsigned char)s[i])-32,
                  false, scalex, scaley, 0.0f, c1, c2);
    }
}


//-----------------------------------------------------
void WriteShadedText(unsigned int x, unsigned int y,
                     SpriteBatcher& pics,
                     int font, const char* s,
                     float scalex, float scaley,
                     COLOR c1,
                     COLOR c2,
                     COLOR shade)
{
    WriteText(x - 1, y - 1, pics, font, s, scalex, scaley, shade, shade);
    WriteText(x, y, pics, font, s, scalex, scaley, c1, c2);

}
//-----------------------------------------------
void WriteSmallShadedText(unsigned int x, unsigned int y,
                     SpriteBatcher& pics,
                     int font, const char* s,
                     float scalex, float scaley,
                     COLOR c1,
                     COLOR c2,
                     COLOR shade)
{
    WriteSmallText(x - 1, y - 1, pics, font, s, scalex, scaley, shade, shade);
    WriteSmallText(x, y, pics, font, s, scalex, scaley, c1, c2);

}

//------------------------------------------------
void DrawNumber(int x, int y, int number, SpriteBatcher& pics, unsigned index)
{
    int arr[7] = {0};

    int i = 6;

    while (number != 0)
    {
        arr[i] = number % 10;
        number = number / 10;
        i--;
    }

    for (int a=0; a < 7; a++)
    {
        pics.draw(index, x + a * 16, y, arr[a], false, 0.98f,0.98f, 0, COLOR(1,1,1,0.6f), COLOR(1,1,1,0.6f));
    }



}

