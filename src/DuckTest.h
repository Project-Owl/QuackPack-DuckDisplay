#pragma once
#ifndef DUCKTEST_H
#define DUCKTEST_H
#include <Wire.h>
#include <string>
#include <QuackPack.h>

template <class  Display>
class DuckTest : public QuackPack {
public:
    DuckTest()
        : width(128),
          height(64),
          reset_pin(-1),
          sda(21),
          scl(22),
          i2caddr(0x3C),
          duckType(DuckType::MAMA) {

    }
    DuckTest(int width, int height, int rst_pin = -1) :
          width(width),
          height(height),
          reset_pin(rst_pin),
          sda(21),
          scl(22),
          i2caddr(0x3C) {

    }

    DuckTest(int width, int height, int sda, int scl, uint8_t i2caddr, int rst_pin = -1)
        : width(width),
          height(height),
          reset_pin(rst_pin),
          sda(sda),
          scl(scl),
          i2caddr(i2caddr) {

    }

    virtual ~DuckTest() = default;

    Display display;

    /**
     * @param none
     * @return void
     * @brief Displays the default screen on the OLED display
     */
    virtual void showDefaultScreen() = 0;
    /**
     * @return void
     * @brief Initializes the OLED display. override to customize initialization. Will call showDefaultScreen after
     * initialization, so you must override that method if using a different display library.
     */

    void launch() override = 0;
    void execute() override { //add text replaced argument?? (for dms)
        //print the last message sent maybe?
        //call displayToScreen
      }
    /**
     * @param none
     * @return void
     * @brief Clears the display
     */
    virtual void clear() = 0;
    /**
     * @param none
     * @return void
     * @brief Puts the display to sleep. Requires wake() to turn back on, as well as rst_pin to be set.
     */
    virtual void sleep() = 0;
    /**
     * @param none
     * @return void
     * @brief Wakes the display from sleep
     */
    virtual void wake() = 0;

    // void displayToScreen(){ 
        //a function that takes a display object, clears the screen, and prints new?
    // }

    enum DuckType {
        /// A Duck of unknown type
        UNKNOWN = 0x00,
        /// A PapaDuck
        PAPA = 0x01,
        /// A MamaDuck
        MAMA = 0x02,
        /// A DuckLink
        LINK = 0x03,
        /// A Detector Duck
        DETECTOR = 0x04,
        MAX_TYPE
      };

    [[nodiscard]] int getWidth() const {return width;}
    [[nodiscard]] int getHeight() const {return height;}


protected:
    int width, height, reset_pin, sda, scl;
    uint8_t i2caddr;
    DuckType duckType;
};
#endif