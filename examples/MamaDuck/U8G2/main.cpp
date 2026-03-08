// Created by brent on 2/20/2026.
#include "U8G2Display.h"
#include "logo.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    // 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SCL_PIN 22 //15 heltec v2
 U8G2_SSD1306_128X64_NONAME_F_HW_I2C driver(U8G2_R0,U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
U8G2Display<U8G2_SSD1306_128X64_NONAME_F_HW_I2C> disp(SCREEN_WIDTH, SCREEN_HEIGHT,driver, U8X8_PIN_NONE);

void setup() {
    // Initialize the display before first use and verify it succeeded.
    disp.setLogo(LOGO);
    disp.launch();
    disp.showLogo(disp.XBM);
    delay(3000);
        disp.display.setFont(u8g2_font_ncenB08_tr);
        disp.display.setCursor(0,0);
        disp.showDefaultScreen();
        delay(3000);

}

void loop() {

    disp.clear();					// clear the internal memory
    disp.display.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    disp.display.drawStr(0,10,"Hello World!");	// write something to the internal memory
    disp.display.sendBuffer();					// transfer internal memory to the display
    disp.display.display();
    delay(1000);
}