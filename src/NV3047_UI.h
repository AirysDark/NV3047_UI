#pragma once
#include <stdint.h>
#include <NV3047.h>

struct UIRect {
    int16_t x, y, w, h;
    UIRect(int16_t px=0, int16_t py=0, int16_t pw=0, int16_t ph=0) : x(px), y(py), w(pw), h(ph) {}
    bool contains(int16_t px, int16_t py) const { return px >= x && py >= y && px < x + w && py < y + h; }
};

namespace UIColor {
    constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
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
    virtual void commit() = 0;
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) = 0;
    virtual bool getTouch(uint16_t& x, uint16_t& y) = 0;
    virtual uint16_t width() const { return 480; }
    virtual uint16_t height() const { return 272; }
};

class NV3047_Adapter : public DisplayDriverInterface {
    NV3047* hardware;
public:
    NV3047_Adapter();
    bool begin(NV3047* hw, bool initializeHardware=true);
    void setBrightness(uint8_t brightness);
    void clear(uint16_t color) override;
    void commit() override;
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* bitmap) override;
    bool getTouch(uint16_t& x, uint16_t& y) override;
    uint16_t width() const override;
    uint16_t height() const override;
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
    static const uint8_t MAX_WIDGETS = 32;
private:
    struct WidgetSlot { Widget* widget; uint8_t z; };
    WidgetSlot widgets[MAX_WIDGETS];
    uint8_t widgetCount;
    Widget* focusedWidget;
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

class NV3047_UI {
    DisplayDriverInterface* display;
    NV3047_Adapter nvAdapter;
    Screen* activeScreen;
    TextRenderer textRenderer;
    UITheme activeTheme;
    uint32_t lastInteractionTime;
    bool wasTouched;
    uint16_t lastTouchX, lastTouchY;
public:
    NV3047_UI();
    bool begin(DisplayDriverInterface* driver);
    bool begin(NV3047* hardware, bool initializeHardware=true);
    void loadScreen(Screen* screen);
    Screen* getActiveScreen() const;
    void setTheme(const UITheme& theme);
    const UITheme& theme() const;
    void setBackgroundColor(uint16_t color);
    void setBrightness(uint8_t brightness);
    uint16_t width() const;
    uint16_t height() const;
    uint32_t getIdleTimeMs() const;
    TextRenderer& text();
    DisplayDriverInterface* driver();
    void drawText(const char* value, int16_t x, int16_t y, uint16_t color, uint8_t scale=1);
    void update();
};
