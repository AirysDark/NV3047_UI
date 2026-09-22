#include <Arduino.h>

#include "NV3047_UI.h"
#include "Page_Matrices/Screen.h"
#include "Renderer/TextRenderer.h"
#include "Font/Font.h"



NV3047_UI::NV3047_UI()
{
    hardware = nullptr;
    active_screen = nullptr;
    bg_color = 0x0000;
    last_interaction_time = 0;
}



bool NV3047_UI::begin(
    DisplayDriverInterface* generic_driver
)
{

    if(!generic_driver)
        return false;


    hardware = generic_driver;


    static Font defaultFont;


    textRenderer.begin(
        hardware,
        &defaultFont
    );


    return true;

}



void NV3047_UI::loadScreen(
    Screen* screen
)
{

    if(!screen)
        return;


    active_screen = screen;


    active_screen->focus();

}



void NV3047_UI::setBackgroundColor(
    uint16_t color
)
{

    bg_color = color;

}



uint32_t NV3047_UI::getIdleTimeMs() const
{

    return millis() - last_interaction_time;

}



void NV3047_UI::drawText(
    const char* text,
    int16_t x,
    int16_t y,
    uint16_t color,
    uint8_t scale
)
{

    if(!hardware)
        return;


    textRenderer.drawText(
        text,
        x,
        y,
        color,
        scale
    );

}



void NV3047_UI::update()
{

    if(!hardware)
        return;



    uint16_t touch_x = 0;
    uint16_t touch_y = 0;



    bool touched =
        hardware->getTouch(
            touch_x,
            touch_y
        );



    if(active_screen)
    {

        if(touched)
        {

            active_screen->handleInput(
                touch_x,
                touch_y
            );


            last_interaction_time = millis();

        }
        else
        {

            active_screen->releaseInput();

        }



        hardware->clear(
            bg_color
        );



        active_screen->render(
            hardware
        );



        hardware->commit();

    }

}