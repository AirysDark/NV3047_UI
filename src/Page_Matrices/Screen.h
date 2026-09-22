#pragma once

#include <stdint.h>
#include "../DisplayDriverInterface.h"


class BaseWidgetReference;


class Screen
{

protected:

    struct WidgetNode
    {
        BaseWidgetReference* widget;
        WidgetNode* next;
        uint8_t z_index;
    };


    WidgetNode* head;

    BaseWidgetReference* focused_widget;

    bool is_dirty;



public:

    Screen();

    virtual ~Screen();



    bool registerWidget(
        BaseWidgetReference* widget,
        uint8_t z_priority = 0
    );


    bool unregisterWidget(
        BaseWidgetReference* widget
    );



    virtual void onInit()
    {
    }


    virtual void onFocus();

    virtual void onDefocus();



    void processInput(
        uint16_t touch_x,
        uint16_t touch_y,
        bool is_touched
    );


    void forceRedraw()
    {
        is_dirty = true;
    }


    bool needsRepaint() const
    {
        return is_dirty;
    }



    void drawScreen(
        DisplayDriverInterface* gfx
    );



    virtual void focus()
    {
        onFocus();
    }



    virtual void handleInput(
        uint16_t touch_x,
        uint16_t touch_y
    )
    {
        processInput(
            touch_x,
            touch_y,
            true
        );
    }



    virtual void releaseInput()
    {
        processInput(
            0,
            0,
            false
        );
    }



    virtual void render(
        DisplayDriverInterface* gfx
    )
    {

        is_dirty = true;

        drawScreen(
            gfx
        );

    }

};