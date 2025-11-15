#include "Intro.h"
#include "SpriteBatcher.h"
#include "SysConfig.h"
#include "gui/Text.h"

Intro::Intro()
{
    memset(IntroText, 0, sizeof(IntroText));
    reset();
    totalLines = 0;
}

void Intro::reset()
{
    currentLine = 0;
    currentChar = 0;
    gline = 0;
    timer = 0;
}

void Intro::logic()
{
    if (gline < totalLines)
    {
        timer++;

        if (timer == INTRO_SHOW_LETTER_DELAY)
        {
            timer = 0;
            currentChar++;

            if ((IntroText[currentLine][currentChar] == '\0'))
            {
                currentLine++;
                gline++;

                if (currentLine == INTRO_LINES_ON_SCREEN)
                {
                    currentLine = 0;
                }

                currentChar = 0;

            }
        }
    }

}

#ifdef __ANDROID__
void Intro::load(const char* filename, AAssetManager* assman)
#else
void Intro::load(const char* filename)
#endif
{
    puts("Loading intro text...");


#ifdef __ANDROID__
    AAsset* f = nullptr;
    f = AAssetManager_open(assman, filename, AASSET_MODE_BUFFER);
#else
    FILE* f;
    f = fopen(filename,"rt");
#endif

    if (!f)
    {
        printf("bummer,could not open %s\n", filename);
        return;
    }


    int pos = 0;
    int c = ';';
    while (c != EOF)
    {
#ifdef __ANDROID__
        int bytesRead = AAsset_read(f, &c, 1);
        if (bytesRead == 0)
        {
            c = EOF;
        }
#else
        c = fgetc(f);
#endif
        if ((c != EOF) || (totalLines < INTRO_TEXT_LINE_COUNT))
        {
            if ((c != '\n') && (pos < INTRO_TEXT_LINE_WIDTH))
            {
                IntroText[totalLines][pos] = c;
                pos++;
            }
            else
            {
                IntroText[totalLines][pos]='\0';
                totalLines++;
                pos = 0;
            }

        }
    }
#ifdef __ANDROID__
    AAsset_close(f);
#else
    fclose(f);
#endif

}


void Intro::draw(SpriteBatcher& pics, const SystemConfig& sys)
{
    pics.draw(13, 320, 180, 0, true);

    char buf[2];

    for (int i = 0; i < currentLine; i++)
    {
        WriteShadedText(30, 25 * i + 20, pics, 10, IntroText[i + INTRO_LINES_ON_SCREEN * (gline / INTRO_LINES_ON_SCREEN)]);
    }

    for (int a = 0; a < currentChar; a++)
    {
        sprintf(buf, "%c", IntroText[gline][a]);
        WriteShadedText(
                  30 + a * 11, 
                  25 * currentLine + 20,
                  pics,
                  10,
                  buf
                  );
    }

    WriteShadedText(30, sys.ScreenHeight - 20, pics, 10, "hit RETURN to skip ...");
}
