#pragma once
#ifndef DUCKDISPLAY_H
#define DUCKDISPLAY_H
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "utils/DuckLogger.h"

template <class  Display = Adafruit_SSD1306>
class DuckDisplay {
public:
    DuckDisplay() : width(128), height(64), sda(21), scl(22) {
        Wire.begin(sda, scl);
        display = Display(width, height, &Wire, -1);
    };
    DuckDisplay(int width, int height) : width(width), height(height), sda(21), scl(22) {
        Wire.begin(sda, scl);
        display = Display(width, height, &Wire, -1);
    }
    DuckDisplay(int width, int height, int sda, int scl, int rst_pin = -1) : width(width), height(height), sda(sda), scl(scl) {
        Wire.begin(sda, scl);
        display = Display(width, height, &Wire, rst_pin);
    }
    void showDefaultScreen();
    void begin() {
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            logdbg_ln("SSD1306 allocation failed");
        }
        showDefaultScreen();
    }
    void clear() {
        display.clearDisplay();
    }
    void sleep() {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    void wake() {
        display.ssd1306_command(SSD1306_DISPLAYON);
    }

    [[nodiscard]] std::uint8_t getWidth() const {return width;}
    [[nodiscard]] std::uint8_t getHeight() const {return height;}
protected:
    std::uint8_t width, height;
private:
    int sda, scl;
    Display display;
};
#endif