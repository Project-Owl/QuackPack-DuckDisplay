#pragma once
#ifndef DUCKTEST_H
#define DUCKTEST_H
#include <Adafruit_SSD1306.h>
#include "utils/DuckLogger.h"
#include <string>
#include <QuackPack.h>

template <class  Display = Adafruit_SSD1306>
class DuckTest : public QuackPack {
public:
    DuckTest(int width, int height, int reset_pin = -1) : width(width), height(height), display(width, height, &Wire, reset_pin) {}
    virtual ~DuckTest() = default;

    Display display;

    /**
     * @param none
     * @return void
     * @brief Displays the default screen on the OLED display
     */
    virtual void showDefaultScreen(){
        this->display.clearDisplay();
        this->display.setTextSize(1);
        this->display.setTextColor(SSD1306_WHITE);
        this->display.setCursor(0, 10);
        this->display.println("MamaDuck Device");
        this->display.println("Initializing...");
        this->display.display();
    }
    /**
     * @return void
     * @brief Initializes the OLED display. override to customize initialization. Will call showDefaultScreen after
     * initialization, so you must override that method if using a different display library.
     */

    void launch() override { //maybe we can rename to begin since it's standard
        if(!this->display.begin(SSD1306_SWITCHCAPVCC, reset_pin))
            logerr_ln("SSD1306 allocation failed");
        Serial.println("setup complete"); //display owl logo?
      }
      void execute() override { //add text replaced argument?? (for dms)
        //print the last message sent maybe?
        //call displayToScreen
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
    virtual void wake() { //why are these all virtual?
        display.ssd1306_command(SSD1306_DISPLAYON);
    }

    // void displayToScreen(){ 
        //a function that takes a display object, clears the screen, and prints new?
    // }

    [[nodiscard]] int getWidth() const {return width;}
    [[nodiscard]] int getHeight() const {return height;}


protected:
    int width, height, reset_pin;
};
#endif