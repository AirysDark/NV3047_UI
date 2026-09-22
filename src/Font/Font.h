#pragma once

#include <stdint.h>
#include "DisplayDriverInterface.h"


struct Glyph
{

    const uint8_t* bitmap;

    uint8_t width;

    uint8_t height;

    uint8_t advance;

};



class Font
{

public:

    static const Glyph* getGlyph(
        char character
    );


    static void drawGlyph(
        DisplayDriverInterface* display,
        const Glyph* glyph,
        int16_t x,
        int16_t y,
        uint16_t color
    );


    static void drawText(
        DisplayDriverInterface* display,
        const char* text,
        int16_t x,
        int16_t y,
        uint16_t color
    );

};