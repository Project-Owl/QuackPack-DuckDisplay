#include "DuckTest.h"
#include <Wire.h>

#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 50
#define OLED_RESET    -1// 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
DuckTest disp(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
void setup() {
    // Initialize the display before first use and verify it succeeded.
    Wire.begin(SDA_PIN, SLC_PIN); //wire has to begin before we can use display
    disp.launch();
    disp.display.clearDisplay();
    disp.display.setTextSize(1);
    disp.display.setTextColor(SSD1306_WHITE);
    disp.display.setCursor(0,0);
    disp.showDefaultScreen();
    disp.display.display(); //maybe we could use some better naming

}

void loop() {
    delay(3000);
    disp.display.clearDisplay();
    disp.display.setCursor(0,10);
    disp.display.println("Looping");
    disp.display.display();
}