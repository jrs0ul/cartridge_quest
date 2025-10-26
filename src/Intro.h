#pragma once


const int INTRO_TEXT_LINE_WIDTH   = 100;
const int INTRO_TEXT_LINE_COUNT   = 100;
const int INTRO_SHOW_LETTER_DELAY = 5;
const int INTRO_LINES_ON_SCREEN   = 18;

class SpriteBatcher;
class SystemConfig;

class Intro
{
    char IntroText[INTRO_TEXT_LINE_COUNT][INTRO_TEXT_LINE_WIDTH];
    int totalLines;
    int timer;
    int currentLine;
    int gline;
    int currentChar;


public:
    Intro();
    void logic();
    void reset();
    void load(const char* filename);
    void draw(SpriteBatcher& pics, const SystemConfig& sys);
};
