#pragma once

// Included automatically at the end of NV3047_UI.h.
// Extra dashboard/application widgets, still C++11 and Core 2.0.17 friendly.

enum class UIStatus : uint8_t { Neutral, Info, Success, Warning, Danger };

namespace UILayout {
    UIRect inset(const UIRect& rect, int16_t amount);
    UIRect rowCell(const UIRect& rect, uint8_t index, uint8_t count, int16_t gap=0);
    UIRect columnCell(const UIRect& rect, uint8_t index, uint8_t count, int16_t gap=0);
}

namespace UIDraw {
    void line(DisplayDriverInterface* d, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void circle(DisplayDriverInterface* d, int16_t cx, int16_t cy, int16_t radius, uint16_t color);
    void fillCircle(DisplayDriverInterface* d, int16_t cx, int16_t cy, int16_t radius, uint16_t color);
    uint16_t statusColor(const UITheme& theme, UIStatus status);
}

class Panel : public Widget {
    uint16_t customFill, customBorder;
    bool useCustomFill, useCustomBorder, elevated;
    int16_t corner;
public:
    Panel();
    explicit Panel(const UIRect& rect);
    void setFillColor(uint16_t color);
    void useThemeFill();
    void setBorderColor(uint16_t color);
    void useThemeBorder();
    void setElevated(bool value);
    void setCornerRadius(int16_t radius);
    void render(UIContext& ctx) override;
};
typedef Panel Card;

class Separator : public Widget {
    bool vertical;
    uint16_t customColor;
    bool useCustomColor;
public:
    Separator();
    Separator(const UIRect& rect, bool isVertical=false);
    void setColor(uint16_t color);
    void useThemeColor();
    void render(UIContext& ctx) override;
};

class StatusBadge : public Widget {
    const char* label;
    UIStatus status;
public:
    StatusBadge();
    StatusBadge(const char* text, const UIRect& rect, UIStatus initialStatus=UIStatus::Neutral);
    void setText(const char* text);
    void setStatus(UIStatus value);
    UIStatus getStatus() const;
    void render(UIContext& ctx) override;
};

class ValueCard : public Widget {
    const char* title;
    const char* value;
    const char* unit;
    UIStatus status;
    uint8_t valueScale;
public:
    ValueCard();
    ValueCard(const char* titleText, const char* valueText, const char* unitText, const UIRect& rect);
    void setTitle(const char* text);
    void setValue(const char* text);
    void setUnit(const char* text);
    void setStatus(UIStatus value);
    void setValueScale(uint8_t scale);
    void render(UIContext& ctx) override;
};

class Gauge : public Widget {
    const char* label;
    const char* unit;
    uint8_t value, majorTicks;
    UIStatus status;
public:
    Gauge();
    Gauge(const char* text, const char* unitText, const UIRect& rect, uint8_t initialValue=0);
    void setValue(uint8_t newValue);
    uint8_t getValue() const;
    void setStatus(UIStatus value);
    void setMajorTicks(uint8_t count);
    void render(UIContext& ctx) override;
};

typedef void (*UITabCallback)(uint8_t index, void* userData);
class TabBar : public Widget {
public:
    static const uint8_t MAX_TABS = 6;
private:
    const char* labels[MAX_TABS];
    uint8_t tabCount, selected;
    UITabCallback callback;
    void* callbackData;
    int8_t pressedTab;
    int8_t tabFromX(int16_t x) const;
public:
    TabBar();
    explicit TabBar(const UIRect& rect);
    bool setTabs(const char* const* tabLabels, uint8_t count);
    void setSelected(uint8_t index, bool notify=false);
    uint8_t getSelected() const;
    void setOnChange(UITabCallback fn, void* userData=nullptr);
    void render(UIContext& ctx) override;
    void onTouchDown(int16_t x, int16_t y) override;
    void onTouchMove(int16_t x, int16_t y) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};

class ListItem : public Widget {
    const char* title;
    const char* subtitle;
    const char* trailing;
    UIStatus status;
    UIButtonCallback callback;
    void* callbackData;
public:
    ListItem();
    ListItem(const char* titleText, const char* subtitleText, const UIRect& rect);
    void setTitle(const char* text);
    void setSubtitle(const char* text);
    void setTrailing(const char* text);
    void setStatus(UIStatus value);
    void setOnClick(UIButtonCallback fn, void* userData=nullptr);
    void render(UIContext& ctx) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};

class Bitmap : public Widget {
    const uint16_t* pixels;
    int16_t bitmapWidth, bitmapHeight;
public:
    Bitmap();
    Bitmap(const uint16_t* data, int16_t width, int16_t height, const UIRect& rect);
    void setBitmap(const uint16_t* data, int16_t width, int16_t height);
    void render(UIContext& ctx) override;
};

typedef void (*UIDialogCallback)(bool accepted, void* userData);
class Dialog : public Widget {
    const char* title;
    const char* message;
    const char* acceptLabel;
    const char* cancelLabel;
    UIDialogCallback callback;
    void* callbackData;
    UIRect cardBounds, acceptBounds, cancelBounds;
    int8_t pressedAction;
    void recalc();
public:
    Dialog();
    Dialog(const char* titleText, const char* messageText, const UIRect& fullScreenBounds);
    void setText(const char* titleText, const char* messageText);
    void setButtons(const char* acceptText, const char* cancelText);
    void setOnResult(UIDialogCallback fn, void* userData=nullptr);
    void show();
    void hide();
    bool isOpen() const;
    void render(UIContext& ctx) override;
    void onTouchDown(int16_t x, int16_t y) override;
    void onTouchMove(int16_t x, int16_t y) override;
    void onTouchUp(int16_t x, int16_t y, bool inside) override;
};
