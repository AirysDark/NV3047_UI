#pragma once

#include <stdint.h>

#include "../DisplayDriverInterface.h"
#include "../Font/Font.h"



class TextRenderer
{

private:

    DisplayDriverInterface* display;

    Font* font;



public:

    TextRenderer();



    void begin(
        DisplayDriverInterface* gfx,
        Font* activeFont
    );



    void drawText(
        const char* text,
        int16_t x,
        int16_t y,
        uint16_t color,
        uint8_t scale = 1
    );

};