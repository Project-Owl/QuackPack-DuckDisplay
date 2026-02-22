// Created by brent on 1/16/2026.

#include "DuckTest.h"
#include "U8g2lib.h"
#include <utils/DuckLogger.h>
#include "utils/DuckUtils.h"

class U8G2Display : public DuckTest<U8G2> {
public:
    U8G2Display()
        : DuckTest() {
        this->display = U8G2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0,U8X8_PIN_NONE, this->scl, this->sda));
    }
    U8G2Display(int width, int height, int rst_pin = -1)
        : DuckTest (width,
          height,
          rst_pin) {
        this->display = U8G2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0,U8X8_PIN_NONE, this->scl, this->sda));
    }

    U8G2Display(int width, int height, int sda, int scl, uint8_t i2caddr, int rst_pin = -1)
        : DuckTest (width,
          height,
          sda,
          scl,
          i2caddr,
          rst_pin) {
        this->display = U8G2(U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0,U8X8_PIN_NONE, this->scl, this->sda));
    }
    ~U8G2Display() override = default;
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
        //display.printf("DT: %s",duckTypeToString(duckType).c_str());
        //display.setCursor(0, 50);
        display.print("v:");
        display.print(duckutils::getCDPVersion().c_str());
        display.sendBuffer();
        display.display();
    }
    void clear() override {
        display.clearDisplay();
    }
    void launch() override {
        Wire.begin(sda, scl);
        Serial.begin(115200);//u8g2 expects 8 bit address
        display.begin();
        showDefaultScreen();
    }
    void sleep() override {
        display.setPowerSave(1);
    }
    void wake() override {
        display.setPowerSave(0);
    }
};