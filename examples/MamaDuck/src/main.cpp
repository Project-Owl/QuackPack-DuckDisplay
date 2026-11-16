/**
 * @file main.cpp
 * @brief MamaDuck that shows what it does on the board's OLED, with
 * QuackPack-DuckDisplay and ClusterDuck Protocol 5.5.
 *
 * The screen shows the duck's ID and type, a status line, and the last packet
 * it heard or sent: topic, origin or destination, the start of the payload,
 * and RSSI, SNR and hop count for packets it heard.
 *
 * - Like every MamaDuck it relays what it hears; duck.run() does that.
 * - Every SEND_INTERVAL_MS it sends a short text message on topics::sensor.
 * - display.update() runs on every loop() pass and sends at most one row of
 *   tiles per call, so the duck keeps relaying while a frame goes out.
 *
 * The display uses the board's OLED as the CDP board header describes it. On
 * a LilyGO T-Beam the OLED is optional: build with -D ENABLE_DISPLAY (the
 * platformio.ini of this repository does). Without a display, begin() returns
 * false and the duck runs the same.
 */

#include <Arduino.h>
#include <string>
#include <CDP.h>
#include <DuckDisplay.h>

// --- Configuration ---
const char DEVICE_ID[] = "MAMA0001";            // exactly 8 characters, unique in the mesh
const unsigned long SEND_INTERVAL_MS = 30000;    // time between two messages

// --- Globals ---
MamaDuck duck(DEVICE_ID);
DuckDisplay display;   // the board's OLED; nothing happens until begin()
auto timer = timer_create_default();
uint32_t counter = 1;  // number of the next message
bool setupOK = false;

/**
 * @brief Called by duck.run() for every packet the duck hears, before it is
 * relayed (CDP handles route requests and replies itself and doesn't pass
 * them on).
 */
void handleDuckData(CdpPacket packet) {
    display.showReceived(packet);

    const DuckDisplay::PacketInfo& heard = display.lastPacket();
    Serial.printf("[MAMA] Heard %s from %s, hop count %u", heard.topicName.c_str(),
                  heard.peer.c_str(), unsigned(heard.hopCount));
    if (heard.hasSignal) {
        Serial.printf(", %.0f dBm, SNR %.1f dB", heard.rssiDbm, heard.snrDb);
    }
    Serial.println();
}

/**
 * @brief Queue one message for the mesh. Called by the timer.
 * @returns true to keep the timer running
 */
bool sendMessage(void*) {
    std::string message = "Hello from " + std::string(DEVICE_ID) + " C:" + std::to_string(counter);

    int err = duck.sendData(topics::sensor, message);
    if (err == DUCK_ERR_NONE) {
        display.showSent(topics::sensor, message);
        display.setStatus("Running");
        Serial.printf("[MAMA] Queued: %s\n", message.c_str());
        counter++;
    } else {
        display.setStatus("Send failed, rc " + std::to_string(err));
        Serial.printf("[MAMA] sendData failed, rc = %d (%s)\n", err, getDuckErrorString(err));
    }
    return true;
}

void setup() {
    Serial.begin(115200);

    // Display first, so it can show the logo while the rest starts.
    if (display.begin()) {
        display.setDuck(duck);
        display.setStatus("Starting");
        display.showLogo();   // for 2 s once it is drawn, then the status screen
        display.refresh();    // draw it now; in loop(), update() does this
    } else {
        Serial.println("[MAMA] No display; running without one");
    }

    // Starts the LoRa radio. There is no network to join: the duck can send
    // and relay as soon as this returns.
    int err = duck.setupWithDefaults();
    if (err != DUCK_ERR_NONE) {
        Serial.printf("[MAMA] Setup failed, rc = %d (%s)\n", err, getDuckErrorString(err));
        display.setStatus("Setup failed, rc " + std::to_string(err));
        return;
    }

    duck.onReceiveDuckData(handleDuckData);
    timer.every(SEND_INTERVAL_MS, sendMessage);
    display.setStatus("Running");
    setupOK = true;
    Serial.println("[MAMA] Setup OK");
}

void loop() {
    display.update();   // also shows "Setup failed" when setup stopped early
    if (!setupOK) {
        return;
    }
    timer.tick();
    duck.run();         // relays received packets, sends queued ones, calls handleDuckData()
}
