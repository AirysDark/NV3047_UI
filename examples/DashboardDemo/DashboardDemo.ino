#include <Arduino.h>
#include <NV3047_UI.h>

NV3047 hardware;
NV3047_UI ui;

class DashboardScreen : public Screen {
public:
    Label title;
    StatusBadge connection;
    Gauge speed;
    ValueCard battery;
    ValueCard temperature;
    ProgressBar fuel;
    TabBar tabs;
    Button alertButton;
    Dialog alertDialog;

    static const char* tabNames[3];

    DashboardScreen()
        : title("NV3047 DASH", UIRect(14, 8, 230, 28), 2),
          connection("ONLINE", UIRect(360, 9, 106, 26), UIStatus::Success),
          speed("SPEED", "KM/H", UIRect(8, 48, 154, 154), 62),
          battery("BATTERY", "12.8", "V", UIRect(174, 52, 140, 68)),
          temperature("ENGINE", "84", "C", UIRect(326, 52, 140, 68)),
          fuel("FUEL", UIRect(174, 135, 292, 38), 74),
          tabs(UIRect(8, 216, 300, 44)),
          alertButton("ALERT", UIRect(320, 216, 146, 44)),
          alertDialog("SYSTEM ALERT", "CHECK SENSOR INPUT", UIRect(0, 0, 480, 272)) {
        battery.setStatus(UIStatus::Success);
        temperature.setStatus(UIStatus::Warning);
        fuel.setFillColor(UIColor::rgb565(80, 210, 140));
        tabs.setTabs(tabNames, 3);
        tabs.setOnChange(onTabChanged, this);
        alertButton.setAccent(false);
        alertButton.setOnClick(onAlert, this);
        alertDialog.setButtons("OK", "CANCEL");
        alertDialog.setOnResult(onDialogResult, this);

        add(title, 1);
        add(connection, 1);
        add(speed, 1);
        add(battery, 1);
        add(temperature, 1);
        add(fuel, 1);
        add(tabs, 2);
        add(alertButton, 2);
        add(alertDialog, 100);
    }

    static void onAlert(void* data) {
        static_cast<DashboardScreen*>(data)->alertDialog.show();
    }

    static void onDialogResult(bool accepted, void* data) {
        DashboardScreen* self = static_cast<DashboardScreen*>(data);
        self->connection.setText(accepted ? "ACK" : "ONLINE");
        self->connection.setStatus(accepted ? UIStatus::Info : UIStatus::Success);
    }

    static void onTabChanged(uint8_t index, void* data) {
        DashboardScreen* self = static_cast<DashboardScreen*>(data);
        if (index == 0) {
            self->speed.setValue(62);
            self->speed.setStatus(UIStatus::Info);
        } else if (index == 1) {
            self->speed.setValue(88);
            self->speed.setStatus(UIStatus::Success);
        } else {
            self->speed.setValue(24);
            self->speed.setStatus(UIStatus::Warning);
        }
    }

    void onDraw(UIContext& ctx) override {
        ctx.display->fillRect(0, 0, 480, 42, ctx.theme->surface);
        ctx.display->drawHLine(0, 41, 480, ctx.theme->border);
        UIRect gaugeCard(4, 46, 162, 162);
        UIDraw::fillSoftRect(ctx.display, gaugeCard, ctx.theme->surface, 7);
        UIDraw::drawSoftRect(ctx.display, gaugeCard, ctx.theme->border, 7);
    }
};

const char* DashboardScreen::tabNames[3] = { "ROAD", "SPORT", "RAIN" };
DashboardScreen dashboard;

void setup() {
    Serial.begin(115200);
    if (!ui.begin(&hardware, true)) {
        Serial.println("NV3047 UI init failed");
        while (true) delay(1000);
    }
    ui.setTheme(UITheme::dark());
    ui.setBrightness(80);
    ui.loadScreen(&dashboard);
}

void loop() {
    if (!ui.update()) {
        Serial.println("NV3047 frame presentation failed");
        delay(10);
    }
}
