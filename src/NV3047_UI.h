#pragma once
#include <stddef.h>
#include <stdint.h>
#include <NV3047_Driver.h>

struct UIRect {
    int16_t x, y, w, h;
    UIRect(int16_t px=0, int16_t py=0, int16_t pw=0, int16_t ph=0) : x(px), y(py), w(pw), h(ph) {}
    bool contains(int16_t px, int16_t py) const { return px >= x && py >= y && px < x + w && py < y + h; }
};

namespace UIColor {
    // Logical RGB -> the verified physical colour-bank order used by NV3047_drivers.
    constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
        return Config::packPanelColor(r, g, b);
    }

    // Kept for source compatibility. On this panel this is intentionally
    // panel-aware rather than textbook RGB565 packing.
    constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return rgb(r, g, b);
    }
}

struct UITheme {
    uint16_t background, surface, surfaceAlt, border, primary, primaryPressed;
    uint16_t text, muted, success, warning, danger, shadow;
    static UITheme dark();
    static UITheme light();
};

enum class UITextAlign : uint8_t { Left, Center, Right };

class DisplayDriverInterface {
public:
    virtual ~DisplayDriverInterface() = default;
    virtual void clear(uint16_t color) = 0;
    virtual bool commit() = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) = 0;
    virtual bool getTouch(uint16_t& x, uint16_t& y) = 0;
    virtual uint16_t width() const { return 480; }
    virtual uint16_t height() const { return 272; }

    // Optional native-driver diagnostics. Custom display adapters can leave
    // these at their zero defaults.
    virtual uint32_t frameCount() const { return 0; }
    virtual uint32_t lastFrameTimeUs() const { return 0; }
    virtual float presentationFPS() const { return 0.0f; }
    virtual size_t framebufferCount() const { return 0; }
    virtual size_t framebufferSizeBytes() const { return 0; }
    virtual size_t framebufferAllocatedBytes() const { return 0; }
    virtual size_t freeManagedMemoryBytes() const { return 0; }
    virtual size_t largestFreeManagedMemoryBlockBytes() const { return 0; }
};

class NV3047_Adapter : public DisplayDriverInterface {
    NV3047_Driver ownedDriver;
    NV3047_Driver* highLevelDriver;
    NV3047* rawHardware;

    Framebuffer* canvas();
    const Framebuffer* canvas() const;

public:
    NV3047_Adapter();

    // Preferred path: uses the v2 high-level driver and its error propagation.
    bool begin(NV3047* hw, bool initializeHardware=true);

    // Attach to an already initialized NV3047_Driver.
    bool begin(NV3047_Driver* readyDriver);

    void setBrightness(uint8_t percentage);
    void sleep();
    void wake();

    void clear(uint16_t color) override;
    bool commit() override;
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) override;
    bool getTouch(uint16_t& x, uint16_t& y) override;
    uint16_t width() const override;
    uint16_t height() const override;

    uint32_t frameCount() const override;
    uint32_t lastFrameTimeUs() const override;
    float presentationFPS() const override;
    size_t framebufferCount() const override;
    size_t framebufferSizeBytes() const override;
    size_t framebufferAllocatedBytes() const override;
    size_t freeManagedMemoryBytes() const override;
    size_t largestFreeManagedMemoryBlockBytes() const override;
};

class TextRenderer {
    DisplayDriverInterface* display;
public:
    TextRenderer();
    void begin(DisplayDriverInterface* gfx);
    void drawText(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t scale=1);
    void drawTextCentered(const char* text, int16_t centerX, int16_t y, uint16_t color, uint8_t scale=1);
    int16_t textWidth(const char* text, uint8_t scale=1) const;
    int16_t textHeight(uint8_t scale=1) const;
};

struct UIContext {
    DisplayDriverInterface* display;
    TextRenderer* text;
    const UITheme* theme;
};

namespace UIDraw {
    void fillSoftRect(DisplayDriverInterface* d, const UIRect& r, uint16_t color, int16_t corner=4);
    void drawSoftRect(DisplayDriverInterface* d, const UIRect& r, uint16_t color, int16_t corner=4);
}

class Widget {
protected:
    UIRect bounds;
    bool visible, enabled, pressed;
public:
    Widget();
    explicit Widget(const UIRect& rect);
    virtual ~Widget() = default;
    void setBounds(int16_t x, int16_t y, int16_t w, int16_t h);
    void setBounds(const UIRect& rect);
    const UIRect& getBounds() const;
    void setVisible(bool value);
    bool isVisible() const;
    void setEnabled(bool value);
    bool isEnabled() const;
    bool hitTest(int16_t x, int16_t y) const;
    virtual void render(UIContext& ctx) = 0;
    virtual void onTouchDown(int16_t x, int16_t y);
    virtual void onTouchMove(int16_t x, int16_t y);
    virtual void onTouchUp(int16_t x, int16_t y, bool inside);
};

class Label : public Widget {
    const char* value;
    uint8_t scale;
    UITextAlign align;
    uint16_t customColor;
    bool useCustomColor;
public:
    Label();
    Label(const char* text, const UIRect& rect, uint8_t textScale=1);
    void setText(const char* text);
    void setScale(uint8_t value);
    void setAlign(UITextAlign value);
    void setColor(uint16_t color);
    void useThemeColor();
    void render(UIContext& ctx) override;
};

typedef void (*UIButtonCallback)(void* userData);
class Button : public Widget {
    const char* label;
    UIButtonCallback callback;
    void* callbackData;
    uint8_t textScale;
    bool accent;
public:
    Button();
    Button(const char* text, const UIRect& rect);
    void setLabel(const char* text);
    void setOnClick(UIButtonCallback fn, void* userData=nullptr);
    void setTextScale(uint8_t scale);
    void setAccent(bool value);
    void render(UIContext& ctx) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};

typedef void (*UIToggleCallback)(bool value, void* userData);
class Toggle : public Widget {
    const char* label;
    bool value;
    UIToggleCallback callback;
    void* callbackData;
public:
    Toggle();
    Toggle(const char* text, const UIRect& rect, bool initialValue=false);
    void setValue(bool newValue, bool notify=false);
    bool getValue() const;
    void setOnChange(UIToggleCallback fn, void* userData=nullptr);
    void render(UIContext& ctx) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};

class ProgressBar : public Widget {
    const char* label;
    uint8_t value;
    bool showValue;
    uint16_t customFill;
    bool useCustomFill;
public:
    ProgressBar();
    ProgressBar(const char* text, const UIRect& rect, uint8_t initialValue=0);
    void setValue(uint8_t newValue);
    uint8_t getValue() const;
    void setShowValue(bool value);
    void setFillColor(uint16_t color);
    void useThemeFill();
    void render(UIContext& ctx) override;
};

typedef void (*UISliderCallback)(uint8_t value, void* userData);
class Slider : public Widget {
    const char* label;
    uint8_t value;
    UISliderCallback callback;
    void* callbackData;
    void updateFromX(int16_t x, bool notify);
public:
    Slider();
    Slider(const char* text, const UIRect& rect, uint8_t initialValue=0);
    void setValue(uint8_t newValue, bool notify=false);
    uint8_t getValue() const;
    void setOnChange(UISliderCallback fn, void* userData=nullptr);
    void render(UIContext& ctx) override;
    void onTouchDown(int16_t x, int16_t y) override;
    void onTouchMove(int16_t x, int16_t y) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};

class Screen {
public:
    static const uint8_t MAX_WIDGETS = 40;
private:
    struct WidgetSlot { Widget* widget; uint8_t z; };
    WidgetSlot widgets[MAX_WIDGETS];
    uint8_t widgetCount;
    Widget* focusedWidget;
    bool initialized;
public:
    Screen();
    virtual ~Screen() = default;
    bool add(Widget& widget, uint8_t z=0);
    bool registerWidget(Widget* widget, uint8_t z=0);
    bool remove(Widget& widget);
    bool unregisterWidget(Widget* widget);
    void clearWidgets();
    virtual void onInit() {}
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onDraw(UIContext& ctx) { (void)ctx; }
    void focus();
    void defocus();
    void handleInput(uint16_t x, uint16_t y);
    void releaseInput(uint16_t x=0, uint16_t y=0);
    void processInput(uint16_t x, uint16_t y, bool touched);
    void render(UIContext& ctx);
};

struct UIDriverStats {
    uint32_t frameCount;
    uint32_t lastFrameTimeUs;
    float presentationFPS;
    size_t framebufferCount;
    size_t framebufferSizeBytes;
    size_t framebufferAllocatedBytes;
    size_t freeManagedMemoryBytes;
    size_t largestFreeManagedMemoryBlockBytes;
};

class NV3047_UI {
    DisplayDriverInterface* display;
    NV3047_Adapter nvAdapter;
    Screen* activeScreen;
    TextRenderer textRenderer;
    UITheme activeTheme;
    uint32_t lastInteractionTime;
    uint32_t frameCounter;
    uint32_t fpsWindowStart;
    uint16_t measuredFps;
    bool lastPresentOK;
    bool wasTouched;
    uint16_t lastTouchX, lastTouchY;
public:
    NV3047_UI();
    bool begin(DisplayDriverInterface* driver);
    bool begin(NV3047* hardware, bool initializeHardware=true);
    bool begin(NV3047_Driver* driver);
    void loadScreen(Screen* screen);
    Screen* getActiveScreen() const;
    void setTheme(const UITheme& theme);
    const UITheme& theme() const;
    void setBackgroundColor(uint16_t color);
    void setBrightness(uint8_t percentage);
    void sleep();
    void wake();
    uint16_t width() const;
    uint16_t height() const;
    uint32_t getIdleTimeMs() const;
    uint16_t getFPS() const;
    float getPresentationFPS() const;
    bool getDriverStats(UIDriverStats& stats) const;
    bool lastPresentSucceeded() const;
    TextRenderer& text();
    DisplayDriverInterface* driver();
    void drawText(const char* value, int16_t x, int16_t y, uint16_t color, uint8_t scale=1);
    bool update();
};

#include "NV3047_UI_Extras.h"
