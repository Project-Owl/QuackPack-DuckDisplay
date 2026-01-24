#include <Arduino.h>
#include <DuckDisplay.h>

DuckDisplay<Adafruit_SSD1306> display;
void setup() {
    display.clear();
    display.getDisplay().setTextSize(1);
    display.getDisplay().setTextColor(SSD1306_WHITE);
    display.getDisplay().setCursor(0,0);
    display.getDisplay().print("Test");

}

void loop() {
    display.clear();
    display.getDisplay().setCursor(0,10);
    display.getDisplay().print("Looping");
}