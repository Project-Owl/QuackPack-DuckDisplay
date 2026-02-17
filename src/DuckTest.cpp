//
// Created by brent on 1/16/2026.
//
// #include "DuckTest.h"
// #include "U8g2lib.h"
// #include "utils/DuckUtils.h"
//
// class U8G2Display : public DuckTest<U8G2> {
// public:
//     ~U8G2Display() override = default;
//     void showDefaultScreen() override {
//         loginfo_ln("Showing default U8G2 screen");
//         display.clearDisplay();
//         display.setFont(u8g2_font_ncenB08_tr);
//         display.setCursor(0, 10);
//         display.drawStr(0,10,"MamaDuck Device");
//         display.drawStr(0,20,"Initializing...");
//         display.sendBuffer();
//         display.clearBuffer();
//         display.setCursor(0, 10);
//         display.print("ClusterDuck");
//         display.setCursor(0,20);
//         display.print("Protocol");
//         display.setCursor(0,30);
//         display.print("----------------");
//         display.setCursor(0, 40);
//         //Not sure if this will call the Print.h or u8g2 print method
//         display.printf("DT: %s",duckTypeToString(duckType).c_str());
//         display.setCursor(0, 50);
//         display.print("v:");
//         display.print(duckutils::getCDPVersion().c_str());
//         display.sendBuffer();
//     }
//     void clear() override {
//         display.clearDisplay();
//     }
//     void begin(uint8_t i2c_addr) override {
//         this->display = U8G2();
//         display.setI2CAddress(i2c_addr);
//         display.begin();
//         showDefaultScreen();
//     }
//     void sleep() override {
//         display.setPowerSave(1);
//     }
//     void wake() override {
//         display.setPowerSave(0);
//     }
// };