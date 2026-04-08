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
    delay(3000);
    driver.display.setTextSize(1);
    driver.display.setTextColor(SSD1306_WHITE);
    driver.display.setCursor(0,0);
    driver.showDefaultScreen();
    driver.display.display(); //maybe we could use some better naming
    delay(3000);

}

void loop() {

    driver.clear();
    delay(3000);
    driver.display.setCursor(0,0);
    driver.display.println("Looping");
    driver.display.display();

}