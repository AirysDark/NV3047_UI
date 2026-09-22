#include "Screen.h"
#include <stdlib.h>


class BaseWidgetReference
{

public:

    virtual void renderWidget(
        DisplayDriverInterface* gfx
    ) = 0;


    virtual bool evaluateHitBox(
        uint16_t x,
        uint16_t y
    ) = 0;


    virtual void onTouchDown()
    {
    }


    virtual void onTouchMove(
        uint16_t x,
        uint16_t y
    )
    {
    }


    virtual void onTouchUp()
    {
    }


    virtual ~BaseWidgetReference()
    {
    }

};



Screen::Screen()
{
    head = nullptr;
    focused_widget = nullptr;
    is_dirty = true;
}



Screen::~Screen()
{

    WidgetNode* current = head;


    while(current)
    {

        WidgetNode* next = current->next;

        delete current;

        current = next;

    }

}



bool Screen::registerWidget(
    BaseWidgetReference* widget,
    uint8_t z_priority
)
{

    if(!widget)
        return false;


    WidgetNode* node =
    new WidgetNode
    {
        widget,
        nullptr,
        z_priority
    };


    is_dirty = true;



    if(!head || head->z_index > z_priority)
    {

        node->next = head;

        head = node;

        return true;

    }



    WidgetNode* current = head;


    while(
        current->next &&
        current->next->z_index <= z_priority
    )
    {

        current = current->next;

    }



    node->next = current->next;

    current->next = node;


    return true;

}



bool Screen::unregisterWidget(
    BaseWidgetReference* widget
)
{

    if(!widget)
        return false;


    WidgetNode* current = head;

    WidgetNode* previous = nullptr;



    while(current)
    {

        if(current->widget == widget)
        {

            if(previous)
                previous->next = current->next;
            else
                head = current->next;



            if(focused_widget == widget)
                focused_widget = nullptr;



            delete current;


            is_dirty = true;


            return true;

        }



        previous = current;

        current = current->next;

    }


    return false;

}



void Screen::onFocus()
{

    is_dirty = true;

}



void Screen::onDefocus()
{

    focused_widget = nullptr;

}



void Screen::processInput(
    uint16_t touch_x,
    uint16_t touch_y,
    bool is_touched
)
{

    if(!is_touched)
    {

        if(focused_widget)
        {

            focused_widget->onTouchUp();

            focused_widget = nullptr;

            is_dirty = true;

        }

        return;

    }



    if(focused_widget)
    {

        focused_widget->onTouchMove(
            touch_x,
            touch_y
        );

        return;

    }



    WidgetNode* current = head;

    BaseWidgetReference* hit = nullptr;



    while(current)
    {

        if(
            current->widget &&
            current->widget->evaluateHitBox(
                touch_x,
                touch_y
            )
        )
        {

            hit = current->widget;

        }


        current = current->next;

    }



    if(hit)
    {

        focused_widget = hit;

        focused_widget->onTouchDown();

        is_dirty = true;

    }

}



void Screen::drawScreen(
    DisplayDriverInterface* gfx
)
{

    if(!gfx)
        return;



    WidgetNode* current = head;



    while(current)
    {

        if(current->widget)
        {

            current->widget->renderWidget(
                gfx
            );

        }


        current = current->next;

    }



    is_dirty = false;

}