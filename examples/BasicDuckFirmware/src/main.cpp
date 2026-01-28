#include <DuckDisplay.h>

//DuckDisplay<Adafruit_SSD1306> display;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
DuckDisplay display(SCREEN_WIDTH, SCREEN_HEIGHT, 21, 22, OLED_RESET);
void setup() {
    // Initialize the display before first use and verify it succeeded.
    display.begin(OLED_RESET);
    display.getDisplay().clearDisplay();
    display.getDisplay().setTextSize(1);
    display.getDisplay().setTextColor(SSD1306_WHITE);
    display.getDisplay().setCursor(0,0);
    display.getDisplay().print("Test");
    display.getDisplay().display();

}

void loop() {
    delay(1000);
    display.getDisplay().clearDisplay();
    display.getDisplay().setCursor(0,10);
    display.getDisplay().println("Looping");
    display.getDisplay().display();
}