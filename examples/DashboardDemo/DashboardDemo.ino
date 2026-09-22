#include <Arduino.h>
#include <NV3047_UI.h>

NV3047 hardware;
NV3047_UI ui;

class DashboardScreen : public Screen {
public:
    Label title;
    Label subtitle;
    ProgressBar battery;
    ProgressBar temperature;
    Toggle lights;
    Slider brightness;
    Button action;
    bool running;

    DashboardScreen()
        : title("NV3047 UI", UIRect(18, 12, 250, 28), 2),
          subtitle("MOTORCYCLE DASH", UIRect(20, 42, 220, 18), 1),
          battery("BATTERY", UIRect(22, 82, 200, 34), 82),
          temperature("ENGINE TEMP", UIRect(258, 82, 200, 34), 64),
          lights("LIGHTS", UIRect(22, 136, 200, 34), false),
          brightness("BRIGHTNESS", UIRect(258, 136, 200, 40), 80),
          action("START", UIRect(22, 202, 436, 48)),
          running(false) {

        title.setAlign(UITextAlign::Left);
        subtitle.setColor(UIColor::rgb565(120, 200, 255));
        temperature.setFillColor(UIColor::rgb565(255, 140, 50));
        action.setTextScale(2);
        action.setOnClick(onAction, this);
        brightness.setOnChange(onBrightness, this);

        add(title, 1);
        add(subtitle, 1);
        add(battery, 1);
        add(temperature, 1);
        add(lights, 1);
        add(brightness, 1);
        add(action, 1);
    }

    static void onAction(void* data) {
        DashboardScreen* self = static_cast<DashboardScreen*>(data);
        self->running = !self->running;
        self->action.setLabel(self->running ? "STOP" : "START");
        self->action.setAccent(!self->running);
    }

    static void onBrightness(uint8_t value, void*) {
        ui.setBrightness((uint8_t)((uint16_t)value * 255 / 100));
    }

    void onDraw(UIContext& ctx) override {
        UIRect header(0, 0, 480, 66);
        UIDraw::fillSoftRect(ctx.display, header, ctx.theme->surface, 0);
        ctx.display->drawHLine(0, 65, 480, ctx.theme->border);
    }
};

DashboardScreen dashboard;

void setup() {
    Serial.begin(115200);

    if (!ui.begin(&hardware, true)) {
        Serial.println("NV3047 UI init failed");
        while (true) delay(1000);
    }

    ui.setTheme(UITheme::dark());
    ui.setBrightness(200);
    ui.loadScreen(&dashboard);
}

void loop() {
    ui.update();
}
