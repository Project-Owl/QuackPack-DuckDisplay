#include "AdafruitDisplay.h"
#include "logo.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1// 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
AdafruitDisplay driver(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
void setup() {
    // Initialize the display before first use and verify it succeeded.
    driver.setLogo(LOGO);
    delay(750); //there's a race condition on these two calls
    driver.launch();
    driver.showLogo(AdafruitDisplay::BITMAP);
    driver.display.setTextSize(1);
    driver.display.setTextColor(SSD1306_WHITE);
    driver.display.setCursor(0,0);
    driver.showDefaultScreen();
    delay(3000);

}

void loop() {

    driver.clear();
    delay(3000);
    driver.display.setCursor(0,0);
    CdpPacket packet;
    packet.dduid = {0x4d,0x46,0x03,0x50,0x44,0x34,0x48,0x44};
    packet.sduid = {0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
    packet.muid = {0x11,0x12,0x13,0x14};
    packet.topic = 1;
    packet.data = {0x15,0x16,0x17,0x18};
    packet.duckType = static_cast<uint8_t>(DuckType::PAPA);
    packet.hopCount = 0;
    packet.timeReceived = 1234567890;
    driver.printPacketHeader(packet);

}