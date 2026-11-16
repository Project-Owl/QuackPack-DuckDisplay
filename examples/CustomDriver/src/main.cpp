/**
 * @file main.cpp
 * @brief MamaDuck with an external OLED that the sketch sets up itself, with
 * QuackPack-DuckDisplay and ClusterDuck Protocol 5.5.
 *
 * DuckDisplay(U8G2&) takes any U8g2 driver. This sketch uses a 1.3" SH1106
 * module on the ESP32's Wire pins (SDA GPIO21, SCL GPIO22), turned 180
 * degrees. Use a full-buffer driver (_F_ in the class name) so that update()
 * can send a frame in parts; https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
 * lists them all.
 *
 * Hardware I2C (_HW_I2C) shares the bus with other Wire devices, such as the
 * T-Beam's PMU. Software I2C (_SW_I2C) needs pins of its own.
 */

#include <Arduino.h>
#include <string>
#include <CDP.h>
#include <DuckDisplay.h>

// --- Configuration ---
const char DEVICE_ID[] = "MAMAOLED";   // exactly 8 characters, unique in the mesh
const uint8_t OLED_SDA = 21;
const uint8_t OLED_SCL = 22;

// --- Globals ---
// Hardware I2C constructor: rotation, reset pin, clock (SCL), data (SDA).
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
DuckDisplay display(oled);
MamaDuck duck(DEVICE_ID);
bool setupOK = false;

void handleDuckData(CdpPacket packet) {
    display.showReceived(packet);
}

void setup() {
    Serial.begin(115200);

    // Driver settings go before begin(), for example a module at 0x3D:
    // oled.setI2CAddress(0x3D * 2);
    if (display.begin()) {
        display.setDuck(duck);
        display.setStatus("Starting");
        display.refresh();
    }

    int err = duck.setupWithDefaults();
    if (err != DUCK_ERR_NONE) {
        Serial.printf("[MAMA] Setup failed, rc = %d (%s)\n", err, getDuckErrorString(err));
        display.setStatus("Setup failed, rc " + std::to_string(err));
        return;
    }

    duck.onReceiveDuckData(handleDuckData);
    display.setStatus("Relaying");
    setupOK = true;
}

void loop() {
    display.update();
    if (!setupOK) {
        return;
    }
    duck.run();
}
