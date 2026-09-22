# NV3047_UI 2.0

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
