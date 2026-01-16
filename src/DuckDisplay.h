#pragma once
#ifndef DUCKDISPLAY_H
#define DUCKDISPLAY_H
#include <cstdint>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

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
    DuckDisplay(int width, int height, int sda, int scl) : width(width), height(height), sda(sda), scl(scl) {
        Wire.begin(sda, scl);
        display = Display(width, height, &Wire, -1);
    }
    void showDefaultScreen();

    std::uint8_t getWidth() const {return width;}
    std::uint8_t getHeight() const {return height;}
protected:
    std::uint8_t width, height;
private:
    int sda, scl;
    Display display;
};
#endif