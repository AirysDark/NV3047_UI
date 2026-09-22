#pragma once

#include <stdint.h>

#include "Renderer/TextRenderer.h"

#include "DisplayDriverInterface.h"


#include "Font/Font.h"




class Screen;



class NV3047_UI
{

private:


    DisplayDriverInterface* hardware;


    Screen* active_screen;


    uint16_t bg_color;


    uint32_t last_interaction_time;



    // ---------------------------------------------
    // Text rendering engine
    // ---------------------------------------------

#if NV3047_ACTIVE_FONT == NV3047_FONT_NORMAL

    Font font;

#elif NV3047_ACTIVE_FONT == NV3047_FONT_POKEMON

    // Pokemon font uses FontManager directly

#endif


    TextRenderer textRenderer;



    // Internal glyph renderer

    void drawGlyph(
        char character,
        int16_t x,
        int16_t y,
        uint16_t color
    );



public:


    NV3047_UI();


    ~NV3047_UI() = default;



    // ---------------------------------------------
    // Core Engine Bindings
    // ---------------------------------------------

    bool begin(
        DisplayDriverInterface* generic_driver
    );



    void loadScreen(
        Screen* screen
    );



    // ---------------------------------------------
    // Framework Control Properties
    // ---------------------------------------------

    void setBackgroundColor(
        uint16_t color
    );



    uint32_t getIdleTimeMs() const;



    // ---------------------------------------------
    // Text Rendering API
    // ---------------------------------------------

	void drawText(
		const char* text,
		int16_t x,
		int16_t y,
		uint16_t color,
		uint8_t scale = 1
	);



    // ---------------------------------------------
    // Main UI update loop
    // ---------------------------------------------

    void update();

};