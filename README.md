# QuackPack-DuckDisplay
A display library guaranteed to work alongside the ClusterDuck Protocol firmware
## Setup
```cpp
#include "AdafruitDisplay.h"
#include "logo.h"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1// 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
AdafruitDisplay driver(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);

void setup() {
    driver.setLogo(LOGO);
    driver.launch();
    driver.showLogo(AdafruitDisplay::BITMAP);
    delay(3000);
    driver.display.setTextSize(1);
    driver.display.setTextColor(SSD1306_WHITE);
    driver.display.setCursor(0,0);
    driver.showDefaultScreen();
    driver.display.display();
}

void loop() {

    driver.clear();
    delay(3000);
    driver.display.setCursor(0,0);
    driver.display.println("Printing to the display!");
    driver.display.display();

}
```
## Using with the U8G2 Library
The U8G2 library supports many different displays, and the `U8G2Display` class can be used to wrap any of the supported U8G2 drivers. Below is an example of using the `U8G2_SSD1306_128X64_NONAME_F_HW_I2C` driver with the `U8G2Display` class.
```cpp
#include "U8G2Display.h"
#include "logo.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    // 16 heltec v2
#define SDA_PIN 21 //4 heltec v2
#define SCL_PIN 22 //15 heltec v2
 U8G2_SSD1306_128X64_NONAME_F_HW_I2C driver(U8G2_R0,U8X8_PIN_NONE, SCL_PIN, SDA_PIN);
U8G2Display<U8G2_SSD1306_128X64_NONAME_F_HW_I2C> disp(SCREEN_WIDTH, SCREEN_HEIGHT,driver, U8X8_PIN_NONE);
```
The rest of the code is similar to the previous example, but note the differences in the `main.cpp` [in the examples directory](examples/MamaDuck/U8G2/main.cpp). 