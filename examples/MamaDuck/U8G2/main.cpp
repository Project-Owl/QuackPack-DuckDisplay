// Created by brent on 2/20/2026.
#include "U8G2Display.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    // 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
U8G2Display disp(SCREEN_WIDTH, SCREEN_HEIGHT, U8X8_PIN_NONE);

void setup() {
    // Initialize the display before first use and verify it succeeded.
    //Wire.begin(SDA_PIN, SLC_PIN); //wire has to begin before we can use display
    disp.launch();
    delay(3000);

}

void loop() {

    disp.display.clearBuffer();					// clear the internal memory
    disp.display.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
    disp.display.drawStr(0,10,"Hello World!");	// write something to the internal memory
    disp.display.sendBuffer();					// transfer internal memory to the display
    delay(1000);
}