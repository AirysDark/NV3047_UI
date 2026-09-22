#include "TextRenderer.h"
#include "../Font/Font.h"



TextRenderer::TextRenderer()
{
    display = nullptr;
    font = nullptr;
}



void TextRenderer::begin(
    DisplayDriverInterface* gfx,
    Font* activeFont
)
{

    display = gfx;
    font = activeFont;

}



void TextRenderer::drawText(
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color,
    uint8_t scale
)
{

    if(
        !display ||
        !font ||
        !text
    )
        return;



    while(*text)
    {

        const Glyph* glyph =
            font->getGlyph(
                *text
            );



        if(glyph)
        {

            for(
                uint8_t col = 0;
                col < glyph->width;
                col++
            )
            {

                uint8_t line =
                    glyph->bitmap[col];



                for(
                    uint8_t row = 0;
                row < glyph->height;
                    row++
                )
                {

                    if(
                        line &
                        (1 << row)
                    )
                    {

                        for(
                            uint8_t sx = 0;
                            sx < scale;
                            sx++
                        )
                        {

                            for(
                                uint8_t sy = 0;
                                sy < scale;
                                sy++
                            )
                            {

                                display->drawPixel(
                                    x + (col * scale) + sx,
                                    y + (row * scale) + sy,
                                    color
                                );

                            }

                        }

                    }

                }

            }



            x += glyph->advance * scale;

        }



        text++;

    }

}