#pragma once
#ifndef DUCKDISPLAY_H
#define DUCKDISPLAY_H
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "utils/DuckLogger.h"
#include "Ducks/DuckTypes.h"
#include "include/cdpcfg.h"

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

    /**
     * @param none
     * @return void
     * @brief Displays the default screen on the OLED display
     */
    virtual void showDefaultScreen()    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("MamaDuck Device");
        display.println("Initializing...");
        display.display();
    }
    /**
     * @param i2c_addr
     * @return void
     * @brief Initializes the OLED display. override to customize initialization. Will call showDefaultScreen after
     * initialization, so you must override that method if using a different display library.
     */
    virtual void begin(uint8_t i2c_addr) {
        if(!display.begin(SSD1306_SWITCHCAPVCC, i2c_addr)) {
            logdbg_ln("SSD1306 allocation failed");
            return;
        }

        display = Display(width, height, &Wire, rst_pin);
        showDefaultScreen();
    }
    /**
     * @param none
     * @return void
     * @brief Clears the display
     */
    virtual void clear() {
        display.clearDisplay();
    }
    /**
     * @param none
     * @return void
     * @brief Puts the display to sleep. Requires wake() to turn back on, as well as rst_pin to be set.
     */
    virtual void sleep() {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    /**
     * @param none
     * @return void
     * @brief Wakes the display from sleep
     */
    virtual void wake() {
        display.ssd1306_command(SSD1306_DISPLAYON);
    }

    [[nodiscard]] std::uint8_t getWidth() const {return width;}
    [[nodiscard]] std::uint8_t getHeight() const {return height;}
    /**
     * @param none
     * @return Display&
     * @brief Returns a reference to the display object for direct manipulation
     */
    Display& getDisplay() {return display;}
    /**
     * @param duckType
     * @return std::string
     * @brief Converts a duck type integer to a human-readable string
     */
    static std::string duckTypeToString(int duckType) {
        std::string duckTypeStr;
        switch (duckType) {
            case DuckType::PAPA:
                duckTypeStr = "Papa";
                break;
            case DuckType::LINK:
                duckTypeStr = "Link";
                break;
            case DuckType::DETECTOR:
                duckTypeStr = "Detector";
                break;
            case DuckType::MAMA:
                duckTypeStr = "Mama";
                break;
            default:
                duckTypeStr = "Duck";
        }
        return duckTypeStr;
    }
protected:
    std::uint8_t width, height;
    int sda, scl, rst_pin, duckType;
    Display display;
};
#endif