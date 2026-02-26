#pragma once
#ifndef DUCKTEST_H
#define DUCKTEST_H
#include <Wire.h>
#include <string>
#include <QuackPack.h>
#include "Ducks/DuckTypes.h"
#include <vector>

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
          duckType(DuckType::UNKNOWN) {

    }
    DuckTest(int width, int height, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : width(width),
          height(height),
          reset_pin(rst_pin),
          sda(21),
          scl(22),
          i2caddr(0x3C),
          duckType(duckType) {

    }

    DuckTest(int width, int height, int sda, int scl, uint8_t i2caddr, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : width(width),
          height(height),
          reset_pin(rst_pin),
          sda(sda),
          scl(scl),
          i2caddr(i2caddr),
          duckType(duckType) {

    }

    virtual ~DuckTest() = default;

    Display display;
    enum IMGTYPE {
        BITMAP,
        XBM
    };

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
     * @param type The type of image data being passed (BITMAP or XBM)
     * @return void
     * @brief Displays the logo on the OLED display. Requires logo to be set first.
     */
    virtual void showLogo(IMGTYPE type) = 0;
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

    virtual void setLogo(const std::vector<std::uint8_t>& logoData) = 0;

    // void displayToScreen(){ 
        //a function that takes a display object, clears the screen, and prints new?
    // }

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

    [[nodiscard]] int getWidth() const {return width;}
    [[nodiscard]] int getHeight() const {return height;}


protected:
    int width, height, reset_pin, sda, scl;
    std::vector<std::uint8_t> logo;
    uint8_t i2caddr;
    DuckType duckType;
};
#endif