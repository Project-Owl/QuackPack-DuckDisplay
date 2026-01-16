//
// Created by brent on 1/16/2026.
//
#include "DuckDisplay.h"
#include <Adafruit_SSD1306.h>

template<class Display>
void DuckDisplay<Display>::showDefaultScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("MamaDuck Device");
    display.println("Initializing...");
    display.display();
}