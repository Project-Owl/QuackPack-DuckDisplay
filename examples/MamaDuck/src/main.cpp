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
 * - On a LilyGO T-Beam v1.2 the status line also shows the battery, read from
 *   the AXP2101 PMU. The PMU and the OLED share GPIO21/22: the sketch starts
 *   Wire for the PMU, and the display uses the same Wire bus (hardware I2C).
 *
 * The display uses the board's OLED as the CDP board header describes it. On
 * a T-Beam the OLED is optional: build with -D ENABLE_DISPLAY (the
 * platformio.ini of this repository does). Without a display, begin() returns
 * false and the duck runs the same.
 *
 * Optional deep sleep, off by default: build with -D ENABLE_DEEP_SLEEP=1 (the
 * lilygo_t_beam_sx1262_deep_sleep environment does). The duck then sleeps for
 * SLEEP_TIME_S after every AWAKE_TIME_MS, and turns the display off first.
 * WARNING: a sleeping MamaDuck receives and relays nothing, so ducks that
 * depend on it lose their path to the PapaDuck. Use it only for a duck that no
 * other duck needs as a relay.
 */

#include <Arduino.h>
#include <string>
#include <CDP.h>
#include <DuckDisplay.h>

#if defined(ARDUINO_T_Beam)
#include <Wire.h>
#include <XPowersLib.h>
#endif

#ifndef ENABLE_DEEP_SLEEP
#define ENABLE_DEEP_SLEEP 0
#endif
#if ENABLE_DEEP_SLEEP
#include <esp_sleep.h>
#endif

// --- Configuration ---
const char DEVICE_ID[] = "MAMA0001";             // exactly 8 characters, unique in the mesh
const unsigned long SEND_INTERVAL_MS = 30000;     // time between two messages
const unsigned long BATTERY_INTERVAL_MS = 10000;  // time between two battery readings (T-Beam)

#if ENABLE_DEEP_SLEEP
const unsigned long AWAKE_TIME_MS = 60000;    // time awake after each boot or wake-up
const unsigned long IDLE_TIMEOUT_MS = 10000;  // longest wait for queued packets to go out
const uint64_t SLEEP_TIME_S = 300;            // time asleep
static_assert(AWAKE_TIME_MS > SEND_INTERVAL_MS, "stay awake long enough to send a message");
#endif

// --- Globals ---
MamaDuck duck(DEVICE_ID);
DuckDisplay display;   // the board's OLED; nothing happens until begin()
auto timer = timer_create_default();
uint32_t counter = 1;  // number of the next message
bool setupOK = false;

std::string state = "Starting";  // first part of the status line
std::string battery;             // second part: "3.98V 87%", or empty

#if defined(ARDUINO_T_Beam)
XPowersAXP2101 pmu;
bool pmuFound = false;
#endif

/**
 * @brief Shows state and battery on the status line. setStatus() only redraws
 * when the text changed.
 */
void showStatus() {
    display.setStatus(battery.empty() ? state : state + "  " + battery);
}

void setState(const std::string& newState) {
    state = newState;
    showStatus();
}

/**
 * @brief T-Beam v1.2: starts Wire on GPIO21/22 and finds the AXP2101 PMU.
 * The OLED is on the same pins; the display shares this Wire bus.
 */
void startPmu() {
#if defined(ARDUINO_T_Beam)
    Wire.begin(21, 22);
    pmuFound = pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, 21, 22);
    if (pmuFound) {
        pmu.enableBattDetection();
        pmu.enableBattVoltageMeasure();
    } else {
        Serial.println("[MAMA] No AXP2101 found (T-Beam v1.0/v1.1 have an AXP192); no battery reading");
    }
#endif
}

/**
 * @brief Reads the battery and updates the status line. Called by the timer.
 * @returns true to keep the timer running
 */
bool readBattery(void*) {
#if defined(ARDUINO_T_Beam)
    if (pmuFound) {
        const uint16_t mv = pmu.getBattVoltage();  // 0 without a battery
        if (mv == 0) {
            battery = "no bat";
        } else {
            char text[16];
            snprintf(text, sizeof(text), "%.2fV %d%%", mv / 1000.0, pmu.getBatteryPercent());
            battery = text;
        }
        showStatus();
    }
#endif
    return true;
}

/**
 * @brief Called by duck.run() for every packet the duck hears, before it is
 * relayed (CDP handles route requests and replies itself and doesn't pass
 * them on).
 */
void handleDuckData(CdpPacket packet) {
    display.showReceived(packet);

    const DuckDisplay::PacketInfo& heard = display.lastPacket();
    Serial.printf("[MAMA] Heard #%lu: %s from %s, hop count %u", (unsigned long)heard.number,
                  heard.topicName.c_str(), heard.peer.c_str(), unsigned(heard.hopCount));
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
        setState("Running");
        Serial.printf("[MAMA] Queued: %s\n", message.c_str());
        counter++;
    } else {
        setState("TX error " + std::to_string(err));
        Serial.printf("[MAMA] sendData failed, rc = %d (%s)\n", err, getDuckErrorString(err));
    }
    return true;
}

void setup() {
    Serial.begin(115200);

    // The PMU first: Wire is then already running when the display starts,
    // and the display joins it.
    startPmu();

    // Then the display, so it can show the logo while the radio starts.
    if (display.begin()) {
        display.setDuck(duck);
        showStatus();
#if ENABLE_DEEP_SLEEP
        // Waking from deep sleep is a reboot; show the logo only on power-up.
        const bool wokeUp = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
#else
        const bool wokeUp = false;
#endif
        if (!wokeUp) {
            display.showLogo();   // for 2 s once it is drawn, then the status screen
        }
        display.refresh();        // draw it now; in loop(), update() does this
    } else {
        Serial.println("[MAMA] No display; running without one");
    }

    // Starts the LoRa radio. There is no network to join: the duck can send
    // and relay as soon as this returns.
    int err = duck.setupWithDefaults();
    if (err != DUCK_ERR_NONE) {
        Serial.printf("[MAMA] Setup failed, rc = %d (%s)\n", err, getDuckErrorString(err));
        setState("Setup error " + std::to_string(err));
        return;
    }

    duck.onReceiveDuckData(handleDuckData);
    timer.every(SEND_INTERVAL_MS, sendMessage);
#if defined(ARDUINO_T_Beam)
    timer.every(BATTERY_INTERVAL_MS, readBattery);
    readBattery(nullptr);
#endif
    setState("Running");
    setupOK = true;
    Serial.printf("[MAMA] Setup OK, display %s, deep sleep %s\n", display.ready() ? "on" : "off",
                  ENABLE_DEEP_SLEEP ? "on" : "off");
}

#if ENABLE_DEEP_SLEEP
void sleepWhenDone();
#endif

void loop() {
    display.update();   // also shows "Setup error" when setup stopped early
    if (!setupOK) {
        return;
    }
    timer.tick();
    duck.run();         // relays received packets, sends queued ones, calls handleDuckData()
#if ENABLE_DEEP_SLEEP
    sleepWhenDone();
#endif
}

#if ENABLE_DEEP_SLEEP
// millis() at which the duck tries to sleep. Waking up is a reboot, so this
// and every other variable start again.
unsigned long sleepAt = AWAKE_TIME_MS;

/**
 * @brief Go to deep sleep once the duck has been awake AWAKE_TIME_MS and has
 * sent what it queued. Called from loop(), so duck.run() keeps running while
 * it waits. The order matters:
 * 1. Wait until duck.txIdle() is true, or give up after IDLE_TIMEOUT_MS.
 * 2. duck.prepareForSleep() puts the LoRa radio to sleep and stops the duck.
 * 3. display.sleep() turns the display off (controller power save).
 * 4. esp_deep_sleep_start(). Waking up is a reboot: setup() runs again.
 */
void sleepWhenDone() {
    const unsigned long now = millis();
    if (now < sleepAt) {
        return;
    }
    const bool idle = duck.txIdle();
    if (!idle && now - sleepAt < IDLE_TIMEOUT_MS) {
        setState("Sleep: sending");
        return;   // keep calling duck.run() until the queues are empty
    }
    if (!idle) {
        Serial.println("[MAMA] Still busy after IDLE_TIMEOUT_MS, dropping what is queued");
    }

    int err = duck.prepareForSleep();
    if (err != DUCK_ERR_NONE) {
        Serial.printf("[MAMA] prepareForSleep failed, rc = %d (%s), staying awake\n", err,
                      getDuckErrorString(err));
        setState("Running");
        sleepAt = now + AWAKE_TIME_MS;   // the duck keeps running; try again later
        return;
    }

    Serial.printf("[MAMA] Sleeping for %lu s\n", (unsigned long)SLEEP_TIME_S);
    Serial.flush();
    display.sleep();
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_S * 1000000ULL);
    esp_deep_sleep_start();
}
#endif
