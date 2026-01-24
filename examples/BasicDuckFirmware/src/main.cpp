//
// Created by brent on 1/16/2026.
//
#include <Arduino.h>
#include <DuckDisplay.h>

DuckDisplay<Adafruit_SSD1306> display;
void setup() {
    display.clear();
    display.getDisplay().print("Test");

}

void loop() {
display.clear();
display.getDisplay().print("Looping");
}