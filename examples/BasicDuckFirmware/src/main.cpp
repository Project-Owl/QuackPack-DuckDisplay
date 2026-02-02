#include <DuckDisplay.h>

//DuckDisplay<Adafruit_SSD1306> disp;
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
DuckDisplay disp(SCREEN_WIDTH, SCREEN_HEIGHT, 21, 22, OLED_RESET);
void setup() {
    // Initialize the display before first use and verify it succeeded.
    disp.begin(OLED_RESET);
    // disp.getDisplay().clearDisplay();
    // disp.getDisplay().setTextSize(1);
    // disp.getDisplay().setTextColor(SSD1306_WHITE);
    // disp.getDisplay().setCursor(0,0);
    disp.showDefaultScreen();
    disp.getDisplay().display();

}

void loop() {
    delay(3000);
    disp.getDisplay().clearDisplay();
    disp.getDisplay().setCursor(0,10);
    disp.getDisplay().println("Looping");
    disp.getDisplay().display();
}