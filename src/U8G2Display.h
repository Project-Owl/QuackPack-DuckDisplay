// Created by brent on 1/16/2026.

#include "DuckTest.h"
#include "U8g2lib.h"
#include <utils/DuckLogger.h>
#include "utils/DuckUtils.h"

template <class Driver>
class U8G2Display : public DuckTest<U8G2> {
public:
    U8G2Display()
        : DuckTest() {
    }

    U8G2Display(Driver& driver)
        : DuckTest() {
        this->display = U8G2(driver);
    }
    U8G2Display(int width, int height,Driver& driver, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : DuckTest (width,
          height,
          rst_pin,
          duckType) {
        this->display = U8G2(driver);
    }

    U8G2Display(int width, int height, int sda, int scl, uint8_t i2caddr,Driver& driver, int rst_pin = -1, DuckType duckType = DuckType::UNKNOWN)
        : DuckTest (width,
          height,
          sda,
          scl,
          i2caddr,
          rst_pin,
          duckType) {
        this->display = U8G2(driver);
    }
    ~U8G2Display() override = default;
    void setDriver(Driver& driver) {
        this->display = U8G2(driver);
    }

    void showDefaultScreen() override {
        loginfo_ln("Showing default U8G2 screen");
        display.clearDisplay();
        display.setFont(u8g2_font_ncenB08_tr);
        display.setCursor(0, 10);
        display.drawStr(0,10,"MamaDuck Device");
        display.drawStr(0,20,"Initializing...");
        display.sendBuffer();
        delay(3000);
        display.clearBuffer();
        display.setCursor(0, 10);
        display.print("ClusterDuck");
        display.setCursor(0,20);
        display.print("Protocol");
        display.setCursor(0,30);
        display.print("----------------");
        display.setCursor(0, 40);
        //Not sure if this will call the Print.h or u8g2 print method
        display.printf("Duck Type: %s",duckTypeToString(duckType).c_str());
        display.setCursor(0, 50);
        display.print("v:");
        display.print(duckutils::getCDPVersion().c_str());
        display.sendBuffer();
        display.display();
    }

    /**
     * @param none
     * @return void
     * @brief Displays the logo on the OLED display. Requires logo to be set first. u8g2 can use either XBM or bitmap.
     */
    void showLogo(IMGTYPE type) override {
        loginfo_ln("Showing logo");
        display.clearBuffer();
        switch (type) {
            case BITMAP:
                display.drawBitmap(0, 0, width, height, logo.data());
                break;
            case XBM:
                display.drawXBM( 0, 0, width, height, logo.data());
                break;
        }
        display.sendBuffer();
    }
    void setLogo(const std::vector<std::uint8_t> &logoData) override {
        loginfo_ln("Setting logo");
        logo = logoData;
    }
    void clear() override {
        display.clearDisplay();
    }
    void launch() override {
        Wire.begin(sda, scl);
        Serial.begin(115200);
        if (!display.begin())
            logerr_ln("U8G2 allocation failed");;
    }
    void sleep() override {
        display.setPowerSave(1);
    }
    void wake() override {
        display.setPowerSave(0);
    }
};