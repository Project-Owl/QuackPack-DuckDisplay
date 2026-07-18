// Created by brent on 2/20/2026.
#ifndef ADAFRUITDISPLAY_H
#define ADAFRUITDISPLAY_H
#include "DuckDisplay.h"
#include <Adafruit_SSD1306.h>
#include "utils/DuckUtils.h"
#include "utils/DuckLogger.h"
#include <string>

class AdafruitDisplay : public DuckDisplay<Adafruit_SSD1306> {
public:
    AdafruitDisplay()
        : DuckDisplay() {
        this->display = Adafruit_SSD1306(width, height, &Wire, reset_pin);
    }
    AdafruitDisplay(int width, int height, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : DuckDisplay (width,
          height,
          rst_pin,
          duckType) {
        this->display = Adafruit_SSD1306(width, height, &Wire, reset_pin);
    }
    AdafruitDisplay(int width, int height, int sda, int scl, uint8_t i2caddr, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : DuckDisplay (width,
          height,
          sda,
          scl,
          i2caddr,
          rst_pin,
          duckType) {
        this->display = Adafruit_SSD1306(width, height, &Wire, reset_pin);
    }

    ~AdafruitDisplay() override = default;

    /**
     * @param none
     * @return void
     * @brief Displays the default screen on the OLED display
     */
    void showDefaultScreen() override {
        std::string str = "Adafruit";

        loginfo("Showing default %s screen \n",str);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 10);
        display.println("Clusterduck");
        display.println("Protocol");
        display.println("----------------");
        display.printf("Duck Type: %s\n",duckTypeToString(duckType).c_str());
        display.print("v:");
        display.print(duckutils::getCDPVersion().c_str());
        display.display();
    }
    /**
     * @return void
     * @brief Initializes the OLED display. override to customize initialization. Will call showDefaultScreen after
     * initialization, so you must override that method if using a different display library.
     */
    void launch() override { //maybe we can rename to begin since it's standard
        Wire.begin(sda, scl);
        Serial.begin(115200);
        if(!this->display.begin(SSD1306_SWITCHCAPVCC, i2caddr))
            logerr_ln("SSD1306 allocation failed");
        loginfo_ln("setup complete"); //display owl logo?
      }
      void execute() override { //add text replaced argument?? (for dms)
        //print the last message sent maybe?
        //call displayToScreen
      }
    /**
     * @param logoData A vector of uint8_t representing the bitmap data for the logo to be displayed. Must be the same dimensions as the display (width*height/8 bytes).
     * @return void
     * @brief Sets the logo to be displayed on the OLED display. Requires showLogo() to actually display, and launch() to initialize the display first.
     */
    void setLogo(const std::vector<std::uint8_t> &logoData) override {
        loginfo_ln("Setting logo");
        logo = logoData;
    };

    /**
     * @brief Displays the logo on the OLED display. Requires logo to be set first.
     * @param type The type of image data being passed (BITMAP or XBM)
     * @param duration The time in time_t to display the logo in milliseconds
     */
    void showLogo(IMGTYPE type, time_t duration=3000) override {
        loginfo_ln("Showing logo");
        display.clearDisplay();
        switch (type) {
            case BITMAP: {
                logdbg_ln("Showing bitmap logo");
                display.drawBitmap(0, 0, logo.data(), width, height, SSD1306_WHITE);
                break;
            }
            case XBM: {
                logdbg_ln("Showing XBM logo");
                display.drawXBitmap(0, 0, logo.data(), width, height, SSD1306_WHITE);
                break;
            }
        }
        display.display();
        delay(duration);
        display.clearDisplay();
    }

    void printPacketHeader(CdpPacket &packet) override {
        logdbg_ln("Printing packet header to display");
        const std::string str = packet.topicToString(); //this method prevents CdpPacket from being passed as const because topicToString is not yet const.
        display.clearDisplay();
        display.setCursor(0, 0);
        // will be replaced with duckutils method when PR is merged.
        display.printf("sduid: %s\n",replaceNonPrintableWithHex(packet.sduid).c_str());
        display.printf("dduid: %s\n",replaceNonPrintableWithHex(packet.dduid).c_str());
        display.printf("muid: %s\n",replaceNonPrintableWithHex(packet.muid).c_str());
        display.printf("topic: %s\n",str.c_str());
        display.printf("duck: %s\n",duckTypeToString(static_cast<DuckType>(packet.duckType)).c_str());
        display.printf("timestamp: %d\n",packet.timeReceived);
        display.display();
    }
    /**
     * @param none
     * @return void
     * @brief Clears the display
     */
    void clear() override {
        display.clearDisplay();
    }
    /**
     * @param none
     * @return void
     * @brief Puts the display to sleep. Requires wake() to turn back on, as well as rst_pin to be set.
     */
    void sleep() override {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
    /**
     * @param none
     * @return void
     * @brief Wakes the display from sleep
     */
    void wake() override { //why are these all virtual?
        display.ssd1306_command(SSD1306_DISPLAYON);
    }

    // void displayToScreen(){
        //a function that takes a display object, clears the screen, and prints new?
    // }
};
#endif