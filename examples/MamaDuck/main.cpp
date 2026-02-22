#include "AdafruitDisplay.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1// 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
AdafruitDisplay disp(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);
void setup() {
    // Initialize the display before first use and verify it succeeded.
    disp.launch();
    //disp.display.clearDisplay();
    disp.display.setTextSize(1);
    disp.display.setTextColor(SSD1306_WHITE);
    disp.display.setCursor(0,0);
    disp.showDefaultScreen();
    disp.display.display(); //maybe we could use some better naming
    delay(3000);

}

void loop() {

    disp.clear();
    delay(3000);
    disp.display.setCursor(0,0);
    disp.display.println("Looping");
    disp.display.display();

}