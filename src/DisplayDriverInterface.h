#pragma once
#include <stdint.h>

class DisplayDriverInterface {
public:
    virtual ~DisplayDriverInterface() = default;
    
    // Universal Frame Execution Controls
    virtual void clear(uint16_t color) = 0;
    virtual void commit() = 0;
    
    // Core Primitives Matrix Requirements
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    
    // High-Speed Block Memory Assets Allocation
    virtual void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) = 0;
    
    // Input Handling Channels
    virtual bool getTouch(uint16_t &x, uint16_t &y) = 0;
};
