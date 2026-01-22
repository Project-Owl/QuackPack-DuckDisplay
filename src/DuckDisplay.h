#pragma once
#ifndef DUCKDISPLAY_H
#define DUCKDISPLAY_H
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "utils/DuckLogger.h"

template <class  Display = Adafruit_SSD1306>
class DuckDisplay {
public:
    DuckDisplay():width(128), height(64), sda(21), scl(22), rst_pin(-1) {
        Wire.begin(sda, scl);
        DuckDisplay<Display>::begin();
    }
    DuckDisplay(int width, int height) : width(width), height(height), sda(21), scl(22), rst_pin(-1) {
        Wire.begin(sda, scl);
        DuckDisplay<Display>::begin();
    }
    DuckDisplay(int width, int height, int sda, int scl, int reset_pin = -1) : width(width), height(height), sda(sda), scl(scl), rst_pin(reset_pin) {
        Wire.begin(sda, scl);
        DuckDisplay<Display>::begin();
    }
    virtual ~DuckDisplay() = default;
    virtual void showDefaultScreen()    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("MamaDuck Device");
        display.println("Initializing...");
        display.display();
    }
    virtual void begin() {
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            logdbg_ln("SSD1306 allocation failed");
        }

        display = Display(width, height, &Wire, rst_pin);
        showDefaultScreen();
    }
    virtual void clear() {
        display.clearDisplay();
    }
    virtual void sleep() {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    virtual void wake() {
        display.ssd1306_command(SSD1306_DISPLAYON);
    }

    [[nodiscard]] std::uint8_t getWidth() const {return width;}
    [[nodiscard]] std::uint8_t getHeight() const {return height;}
protected:
    std::uint8_t width, height;
    int sda, scl, rst_pin;
    Display display;
};
#endif