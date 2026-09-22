# NV3047_UI 2.0

> **Driver overhaul in progress:** `NV3047_drivers` is currently being overhauled and optimized. NV3047_UI is being kept aligned with the existing Core 2.0.17 stack and will be tuned further for the upcoming driver update. Stay tuned for the next driver-side update.

A lightweight native UI toolkit for the **NV3047 / Elecrow 4.3-inch 480x272 ESP32-S3 display stack**.

`NV3047_drivers` stays responsible for the hardware, RGB bus, XPT2046 touch input and double-buffered framebuffer. `NV3047_UI` now sits above that layer and provides reusable screens and controls instead of requiring every application to manually draw rectangles and poll touch coordinates.

## What changed in 2.0

- Direct `NV3047` hardware adapter; no custom glue class required.
- Fixed Arduino library metadata and dependency information.
- Theme system with dark/light presets and RGB565 helper.
- Screen manager with deterministic fixed-size widget storage (no per-widget heap allocations).
- Z-ordered touch hit testing and press/move/release focus handling.
- Improved text renderer with scaling, centering, lowercase handling and common punctuation.
- Reusable `Label`, `Button`, `Toggle`, `ProgressBar`, and `Slider` widgets.
- Lightweight soft-corner drawing helpers for modern cards and controls.
- Full 480x272 dashboard example.

## Architecture

```text
NV3047_UI
├── NV3047_UI                 application/UI lifecycle
├── NV3047_Adapter            bridge to NV3047_drivers
├── Screen                    screen + touch focus manager
├── TextRenderer / Font       lightweight 5x7 text engine
├── UITheme / UIRect          styling and layout primitives
└── Widgets
    ├── Label
    ├── Button
    ├── Toggle
    ├── ProgressBar
    └── Slider
```

## Quick start

```cpp
#include <Arduino.h>
#include <NV3047_UI.h>

NV3047 hardware;
NV3047_UI ui;

class HomeScreen : public Screen {
public:
    Label title;
    Button button;
    Toggle lights;

    HomeScreen()
        : title("MY DASH", UIRect(20, 18, 440, 30), 2),
          button("START", UIRect(20, 190, 440, 50)),
          lights("LIGHTS", UIRect(20, 90, 200, 36)) {
        title.setAlign(UITextAlign::Center);
        button.setOnClick(onStart, this);
        add(title);
        add(lights);
        add(button);
    }

    static void onStart(void*) {
        Serial.println("Start pressed");
    }
};

HomeScreen home;

void setup() {
    Serial.begin(115200);

    if (!ui.begin(&hardware)) {
        while (true) delay(1000);
    }

    ui.setTheme(UITheme::dark());
    ui.setBrightness(200);
    ui.loadScreen(&home);
}

void loop() {
    ui.update();
}
```

## Creating a custom screen

Subclass `Screen`, create widgets as members, then register them with `add(widget, zIndex)`. Widgets are not dynamically allocated by the framework, which keeps behavior predictable on the ESP32-S3.

Override `onDraw(UIContext&)` when a screen needs custom background graphics or decorative sections. The context gives direct access to:

- `ctx.display` - low-level drawing primitives
- `ctx.text` - text renderer
- `ctx.theme` - active theme colors

## Touch behavior

The screen manager captures the topmost enabled widget under the initial touch. That same widget receives move events until release, preventing a dragged finger from accidentally activating a different control. Buttons activate on release while still inside their bounds. Sliders update continuously while dragged.

## Themes

```cpp
ui.setTheme(UITheme::dark());
ui.setTheme(UITheme::light());
```

Custom themes are plain `UITheme` structs. Colors use RGB565. You can create colors with:

```cpp
uint16_t orange = UIColor::rgb565(255, 120, 20);
```

## Included example

Open:

`File -> Examples -> NV3047_UI -> DashboardDemo`

The demo shows a full dark dashboard with header styling, two progress indicators, a toggle, a brightness slider and a large action button.

## Hardware layer

This project expects the `NV3047` Arduino library from:

https://github.com/AirysDark/NV3047_drivers

The UI library does not replace the working native framebuffer or touch driver. It builds on top of them.


## ESP32 core compatibility

The target is **Arduino-ESP32 Core 2.0.17**. The framework deliberately stays within C++11-era features and avoids ESP32 Core 3.x-only APIs, LVGL requirements, STL containers and C++17-only language features.

## Expanded UI 2.1

In addition to Label, Button, Toggle, ProgressBar and Slider, the framework now includes:

- `Panel` / `Card`
- `Separator`
- `StatusBadge`
- `ValueCard`
- `Gauge`
- `TabBar` (up to six tabs)
- `ListItem`
- `Bitmap`
- `Dialog`

The screen capacity is now 40 widgets. `Screen::onInit()` runs once on first load, while `onEnter()` and `onExit()` remain per-navigation hooks. Disabled Buttons, Toggles and Sliders also have distinct muted rendering.

### Layout helpers

```cpp
UIRect content(12, 50, 456, 160);
UIRect left  = UILayout::rowCell(content, 0, 2, 10);
UIRect right = UILayout::rowCell(content, 1, 2, 10);
UIRect top   = UILayout::columnCell(content, 0, 3, 8);
```

### Gauge

```cpp
Gauge speed("SPEED", "KM/H", UIRect(8, 48, 154, 154), 62);
speed.setStatus(UIStatus::Info);
speed.setMajorTicks(6);
```

Gauge values are normalized from 0 to 100, so applications can map speed, RPM, temperature or other sensor ranges into the control.

### Value card and status badge

```cpp
ValueCard battery("BATTERY", "12.8", "V", UIRect(174, 52, 140, 68));
battery.setStatus(UIStatus::Success);

StatusBadge link("ONLINE", UIRect(360, 9, 106, 26), UIStatus::Success);
```

Available status styles are Neutral, Info, Success, Warning and Danger.

### Tabs

```cpp
const char* modes[] = { "ROAD", "SPORT", "RAIN" };
TabBar tabs(UIRect(8, 216, 300, 44));
tabs.setTabs(modes, 3);
tabs.setOnChange(onModeChanged, this);
```

### Modal dialog

Register a dialog at a high z-index. It is hidden by default. When shown, its full-screen bounds capture touch so controls underneath cannot fire.

```cpp
Dialog warning("WARNING", "ENGINE TEMP HIGH", UIRect(0, 0, 480, 272));
warning.setButtons("OK", "CANCEL");
add(warning, 100);
warning.show();
```

### FPS measurement

`ui.getFPS()` returns the measured UI frame rate. This does not replace or alter the NV3047 driver's existing framebuffer cadence logic.

The expanded `DashboardDemo` demonstrates the gauge, value cards, status badge, progress bar, tabs, action button and modal dialog.
