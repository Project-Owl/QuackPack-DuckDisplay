// Created by brent on 2/20/2026.
#include "U8G2Display.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1// 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
U8G2Display disp(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);

void setup() {
    // Initialize the display before first use and verify it succeeded.
    //Wire.begin(SDA_PIN, SLC_PIN); //wire has to begin before we can use display
    disp.launch();
    delay(3000);

}

void loop() {

    disp.clear();
    delay(3000);
    disp.display.setCursor(0,0);
    disp.display.println("Looping");
    disp.display.display();

}