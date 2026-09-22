#include "Font.h"


// --------------------------------------------------
// 5x7 FONT DATA
// Stored as columns
// --------------------------------------------------

static const uint8_t font5x7[][5] =
{

    // SPACE
    {0x00,0x00,0x00,0x00,0x00},


    // A
    {0x7E,0x09,0x09,0x7E,0x00},

    // B
    {0x7F,0x49,0x49,0x36,0x00},

    // C
    {0x3E,0x41,0x41,0x22,0x00},

    // D
    {0x7F,0x41,0x41,0x3E,0x00},

    // E
    {0x7F,0x49,0x49,0x41,0x00},

    // F
    {0x7F,0x09,0x09,0x01,0x00},

    // G
    {0x3E,0x41,0x51,0x32,0x00},

    // H
    {0x7F,0x08,0x08,0x7F,0x00},

    // I
    {0x41,0x7F,0x41,0x00,0x00},

    // J
    {0x20,0x40,0x41,0x3F,0x00},

    // K
    {0x7F,0x08,0x14,0x63,0x00},

    // L
    {0x7F,0x40,0x40,0x40,0x00},

    // M
    {0x7F,0x06,0x06,0x7F,0x00},

    // N
    {0x7F,0x06,0x18,0x7F,0x00},

    // O
    {0x3E,0x41,0x41,0x3E,0x00},

    // P
    {0x7F,0x09,0x09,0x06,0x00},

    // Q
    {0x3E,0x41,0x61,0x7E,0x00},

    // R
    {0x7F,0x09,0x19,0x66,0x00},

    // S
    {0x26,0x49,0x49,0x32,0x00},

    // T
    {0x01,0x7F,0x01,0x01,0x00},

    // U
    {0x3F,0x40,0x40,0x3F,0x00},

    // V
    {0x1F,0x20,0x40,0x3F,0x00},

    // W
    {0x7F,0x30,0x30,0x7F,0x00},

    // X
    {0x63,0x14,0x14,0x63,0x00},

    // Y
    {0x07,0x08,0x70,0x07,0x00},

    // Z
    {0x61,0x51,0x49,0x47,0x00},


    // -
    {0x08,0x08,0x08,0x08,0x00},


    // 0
    {0x3E,0x51,0x49,0x45,0x3E},

    // 1
    {0x00,0x42,0x7F,0x40,0x00},

    // 2
    {0x42,0x61,0x51,0x49,0x46},

    // 3
    {0x21,0x41,0x45,0x4B,0x31},

    // 4
    {0x18,0x14,0x12,0x7F,0x10},

    // 5
    {0x27,0x45,0x45,0x45,0x39},

    // 6
    {0x3C,0x4A,0x49,0x49,0x30},

    // 7
    {0x01,0x71,0x09,0x05,0x03},

    // 8
    {0x36,0x49,0x49,0x49,0x36},

    // 9
    {0x06,0x49,0x49,0x29,0x1E}

};



// SPACE + A-Z + SYMBOLS + DIGITS
static Glyph glyphTable[37];

static bool fontInitialized = false;



static void initFontTable()
{

    if(fontInitialized)
        return;


    for(int i = 0; i < 37; i++)
    {

        glyphTable[i].bitmap =
            font5x7[i];

        glyphTable[i].width =
            5;

        glyphTable[i].height =
            7;

        glyphTable[i].advance =
            6;

    }


    fontInitialized = true;

}



const Glyph* Font::getGlyph(
    char character
)
{

    initFontTable();


    // SPACE
    if(character == ' ')
    {
        return &glyphTable[0];
    }


    // A-Z
    if(character >= 'A' && character <= 'Z')
    {
        return &glyphTable[
            (character - 'A') + 1
        ];
    }


    // -
    if(character == '-')
    {
        return &glyphTable[27];
    }


    // 0-9
    if(character >= '0' && character <= '9')
    {
        return &glyphTable[
            28 + (character - '0')
        ];
    }


    return &glyphTable[0];

}



void Font::drawGlyph(
    DisplayDriverInterface* display,
    const Glyph* glyph,
    int16_t x,
    int16_t y,
    uint16_t color
)
{

    if(!display || !glyph)
        return;


    for(uint8_t col = 0; col < glyph->width; col++)
    {

        uint8_t line =
            glyph->bitmap[col];


        for(uint8_t row = 0; row < glyph->height; row++)
        {

            if(line & (1 << row))
            {

                display->drawPixel(
                    x + col,
                    y + row,
                    color
                );

            }

        }

    }

}



void Font::drawText(
    DisplayDriverInterface* display,
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color
)
{

    if(!display || !text)
        return;


    int16_t cursorX = x;


    while(*text)
    {

        const Glyph* glyph =
            getGlyph(
                *text
            );


        drawGlyph(
            display,
            glyph,
            cursorX,
            y,
            color
        );


        cursorX += glyph->advance;


        text++;

    }

}