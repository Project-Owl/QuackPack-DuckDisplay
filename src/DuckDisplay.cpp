//
// Created by brent on 1/16/2026.
//
#include "DuckDisplay.h"
#include "U8g2lib.h"

class U8G2Display : public DuckDisplay<U8G2> {
public:
    ~U8G2Display() override = default;
    void showDefaultScreen() override {
        display.clearDisplay();
        display.setFont(u8g2_font_ncenB08_tr);
        display.setCursor(0, 10);
        display.println("MamaDuck Device");
        display.println("Initializing...");
        display.sendBuffer();
        display.display();
    }
    void clear() override {
        display.clearDisplay();
    }
    void begin() override {
        display = U8G2();
        display.begin();
        showDefaultScreen();
    }
    void sleep() override {
        display.setPowerSave(1);
    }
    void wake() override {
        display.setPowerSave(0);
    }
};