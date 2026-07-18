#pragma once
#ifndef DUCKTEST_H
#define DUCKTEST_H
#include <iomanip>
#include <sstream>
#include <string>
#include <QuackPack.h>
#include "Ducks/DuckTypes.h"
#include <vector>
#include <CdpPacket.h>

template <class  Display>
class DuckDisplay : public QuackPack {
public:
    DuckDisplay()
        : width(128),
          height(64),
          reset_pin(-1),
          sda(21),
          scl(22),
          i2caddr(0x3C),
          duckType(DuckType::UNKNOWN) {

    }
    DuckDisplay(int width, int height, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : width(width),
          height(height),
          reset_pin(rst_pin),
          sda(21),
          scl(22),
          i2caddr(0x3C),
          duckType(duckType) {

    }

    DuckDisplay(int width, int height, int sda, int scl, uint8_t i2caddr, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : width(width),
          height(height),
          reset_pin(rst_pin),
          sda(sda),
          scl(scl),
          i2caddr(i2caddr),
          duckType(duckType) {

    }

    virtual ~DuckDisplay() = default;

    Display display;
    enum IMGTYPE {
        BITMAP,
        XBM
    };

    /**
     * @param none
     * @return none
     * @brief Displays the default screen on the OLED display
     */
    virtual void showDefaultScreen() = 0;
    /**
     * @return none
     * @brief Initializes the OLED display. override to customize initialization. Calls Serial.begin() with baud 115200.
     */

    void launch() override = 0;
    void execute() override { //add text replaced argument?? (for dms)
        //print the last message sent maybe?
        //call displayToScreen
      }
    /**
     * @brief Displays the logo on the OLED display. Requires logo to be set first.
     * @param type The type of image data being passed (BITMAP or XBM)
     * @param duration The time in time_t to display the logo in milliseconds
     */
    virtual void showLogo(IMGTYPE type, time_t duration) = 0;
    /**
     * @param none
     * @return none
     * @brief Clears the display
     */
    virtual void clear() = 0;
    /**
     * @param none
     * @return none
     * @brief Puts the display to sleep. Requires wake() to turn back on, as well as rst_pin to be set.
     */
    virtual void sleep() = 0;
    /**
     * @param none
     * @return none
     * @brief Wakes the display from sleep
     */
    virtual void wake() = 0;

    /**
     *
     * @param logoData
     * @return none
     * @brief A vector of std::uint8_t representing a logo to be displayed on the OLED display.
     */
    virtual void setLogo(const std::vector<std::uint8_t>& logoData) = 0;

    // void displayToScreen(){ 
        //a function that takes a display object, clears the screen, and prints new?
    // }
    /**
     * @brief Prints the header of a CdpPacket to the display. Override this method to customize how the packet header is displayed.
     * @param packet
     */
    virtual void printPacketHeader(CdpPacket& packet) = 0;
    /**
     * @param duckType
     * @return std::string
     * @brief Converts a duck type integer to a human-readable string
     */
    static std::string duckTypeToString(const DuckType& duckType) {
        std::string duckTypeStr;
        switch (duckType) {
            case DuckType::PAPA: {
                duckTypeStr = "Papa";
                break;
            }
            case DuckType::LINK: {
                duckTypeStr = "Link";
                break;
            }
            case DuckType::DETECTOR: {
                duckTypeStr = "Detector";
                break;
            }
            case DuckType::MAMA: {
                duckTypeStr = "Mama";
                break;
            }
            case DuckType::UNKNOWN: {
                duckTypeStr = "Unknown";
                break;
            }
        }
        return duckTypeStr;
        }
    template<typename Container>
    static std::string replaceNonPrintableWithHex(const Container& input) {
        std::string result;
        for (unsigned char c : input) {
            if (std::isprint(c)) {
                result += static_cast<char>(c);
            } else {
                // Convert non-printable byte to hex string (e.g., 0x0A -> "0A")
                std::ostringstream oss;
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
                result += oss.str();
            }
        }
        return result;
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
