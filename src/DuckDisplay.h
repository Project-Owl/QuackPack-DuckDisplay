#pragma once
#ifndef DUCKDISPLAY_H
#define DUCKDISPLAY_H
#include <cstdint>
#include <Wire.h>

template <class  Display>
class DuckDisplay {
public:
    DuckDisplay() : width(128), height(64) {

    };
    void showDefaultScreen();

    std::uint8_t getWidth() const {return width;}
    std::uint8_t getHeight() const {return height;}
protected:
    std::uint8_t width, height;
private:
    Display display(width, height, &Wire, -1);
};
#endif