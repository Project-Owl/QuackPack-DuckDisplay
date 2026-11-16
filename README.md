# QuackPack-DuckDisplay

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![ClusterDuck Protocol](https://img.shields.io/badge/ClusterDuck-Protocol-yellow.svg)](https://github.com/ClusterDuck-Protocol/ClusterDuck-Protocol)
[![Build examples](https://github.com/Project-Owl/QuackPack-DuckDisplay/actions/workflows/build.yml/badge.svg)](https://github.com/Project-Owl/QuackPack-DuckDisplay/actions/workflows/build.yml)

## Overview

This QuackPack shows what a Duck is doing on its OLED. It draws a status screen with [U8g2](https://github.com/olikraus/u8g2):

- the duck's ID and type;
- a status line that the sketch sets;
- the last packet the duck heard or sent: topic, origin or destination, the start of the payload, and RSSI, SNR and hop count for packets it heard.

It does not block the Duck's `loop()`. `update()` draws the screen into U8g2's buffer, which takes no bus time, and sends one row of tiles (8 pixel rows) per call.

By default it uses the board's OLED as the ClusterDuck Protocol (CDP) board header describes it. ClusterDuck Protocol 5.5 has no display code of its own (it was removed in ClusterDuck-Protocol PR #514); this QuackPack replaces it. Logging goes through the CDP logger.

## What is a QuackPack?

A **QuackPack** is a modular library that extends Duck firmware with application-specific functionality. QuackPacks are **not standalone projects**: they are included in a Duck firmware project and configured from its sketch.

## Hardware

### Supported boards

The default constructor takes the OLED's pins and U8g2 class from the CDP board header (`CDPCFG_PIN_OLED_*`, `CDPCFG_OLED_CLASS`).

| Board (CDP board header) | PlatformIO `board` | OLED | Bus | SDA / SCL / reset | Built in CI |
|---|---|---|---|---|---|
| LilyGO T-Beam SX1262 | `ttgo-t-beam` | SSD1306 128x64, optional | hardware I2C (`Wire`) | 21 / 22 / none | yes |
| Heltec WiFi LoRa 32 V3 | `heltec_wifi_lora_32_V3` | SSD1306 128x64 | software I2C | 17 / 18 / 21 | yes |
| Heltec WiFi LoRa 32 V2 | `heltec_wifi_lora_32_V2` | SSD1306 128x64 | software I2C | 4 / 15 / 16 | yes |
| TTGO LoRa32 V1.3 (`-D TTGO_1_3`) | `ttgo-lora32-v1` | SSD1306 128x64 | software I2C | 4 / 15 / 16 | no |
| T-Beam SX1276 (CDP builds it as `ttgo-lora32-v1`) | `ttgo-lora32-v1` | SSD1306 128x64 | hardware I2C (`Wire`) | 21 / 22 / 16 | no |
| LilyGO T-Beam Supreme | `esp32-s3-devkitc-1`, `-D ARDUINO_LILYGO_T_BEAM_SUPREME` | SH1106 128x64 | software I2C | 17 / 18 / none | no |

Boards without an OLED (LilyGO T-SIM7000G) build and run: `begin()` returns false and the other calls do nothing.

Any other display that U8g2 drives (SSD1306, SH1106, SSD1309, ST7567 and [many more](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp)) works through `DuckDisplay(U8G2&)`; see [examples/CustomDriver](examples/CustomDriver/src/main.cpp).

### Wiring and board notes

- **LilyGO T-Beam**: the OLED is optional, so CDP only defines its pins when the firmware is built with `-D ENABLE_DISPLAY`. Fit a 0.96" SSD1306 I2C module to the OLED header: VCC to 3.3 V, GND, SDA to GPIO21, SCL to GPIO22. Without `ENABLE_DISPLAY`, `begin()` logs a warning and returns false.
- **Boards with the OLED on GPIO21/22** (T-Beam): the AXP192/AXP2101 PMU is on the same pins, and XPowersLib (used by QuackPack-DuckGPS and QuackPack-DuckSleep) talks to it through `Wire`. U8g2's software I2C would take the pins away from `Wire`, so on these boards DuckDisplay uses hardware I2C (`Wire`, 400 kHz) instead of CDP's software-I2C class. Display and PMU share the bus, in either start-up order.
- **Heltec V2 and V3**: the OLED is powered from the Vext rail. `begin()` switches Vext on (GPIO21 on V2, GPIO36 on V3, active low) and waits 100 ms.
- **External modules**: VCC to 3.3 V, GND, SDA and SCL to the pins you pass to the U8g2 constructor. Most SSD1306/SH1106 modules answer at I2C address 0x3C; for 0x3D, call `oled.setI2CAddress(0x3D * 2)` before `begin()`.

## Installation

### Requirements

- [PlatformIO](https://platformio.org/)
- A Duck firmware project based on [ClusterDuck Protocol](https://github.com/ClusterDuck-Protocol/ClusterDuck-Protocol) 5.x (the examples are built against CDP 5.5)

### Pinning it in a project (DMS, CDP or your own)

Add the QuackPack to your project's `lib_deps`. Pin it by **commit SHA** and name the tag in a comment, the same way the project pins CDP. A tag can be moved; a SHA cannot. Name the dependency (`QuackPack-DuckDisplay = ...`) so PlatformIO treats every reference as one package.

```ini
lib_deps =
    ; your project owns the CDP pin
    ClusterDuck-Protocol = https://github.com/Project-Owl/ClusterDuck-Protocol.git#c293c742221999e9b68c29c93c76cb9afece21bd
    ; QuackPack-DuckDisplay v1.1.0
    QuackPack-DuckDisplay = https://github.com/Project-Owl/QuackPack-DuckDisplay.git#<v1.1.0 commit SHA>
build_flags =
    ; LilyGO T-Beam only, when an OLED is fitted
    -D ENABLE_DISPLAY
```

`git rev-list -n 1 v1.1.0` prints the commit of a tag.

The library depends only on `olikraus/U8g2`, which PlatformIO installs automatically.

**It has no ClusterDuck-Protocol dependency on purpose.** A git-URL dependency in a library's `library.json` is re-resolved on every `pio pkg install`, and it replaces the CDP version the project pinned. Version 1.0.0 did this, pointing at a personal fork. Each project pins CDP itself.

With PlatformIO's default library finder (`chain`), only sketches that `#include <DuckDisplay.h>` compile the library, so other ducks in the same project do not grow.

## Quick start

```cpp
#include <CDP.h>
#include <DuckDisplay.h>

MamaDuck duck("MAMA0001");
DuckDisplay display;                 // the board's OLED; nothing happens until begin()

void handleDuckData(CdpPacket packet) {
    display.showReceived(packet);    // topic, origin, payload, RSSI/SNR, hop count
}

void setup() {
    Serial.begin(115200);
    display.begin();                 // false if the board has no OLED; the duck runs anyway
    display.setDuck(duck);           // ID and type
    display.setStatus("Starting");
    display.refresh();               // show it now, while the radio starts

    duck.setupWithDefaults();
    duck.onReceiveDuckData(handleDuckData);
    display.setStatus("Running");
}

void loop() {
    duck.run();
    display.update();                // never sends more than one row of tiles
}
```

To show what the duck sends, call `showSent()` after `sendData()` succeeds:

```cpp
if (duck.sendData(topics::sensor, payload) == DUCK_ERR_NONE) {
    display.showSent(topics::sensor, payload);
}
```

The full sketch is in [examples/MamaDuck](examples/MamaDuck/src/main.cpp).

## API

Include `<DuckDisplay.h>`; the class is `DuckDisplay`.

| Member | Description |
|--------|-------------|
| `DuckDisplay()` | Uses the board's OLED from the CDP board header. Does not touch the hardware. |
| `explicit DuckDisplay(U8G2& oled)` | Uses a U8g2 driver the sketch constructed. It must outlive the `DuckDisplay`. Use a full-buffer driver (`_F_`) so frames go out in parts; with a page-buffer driver (`_1_`, `_2_`) each frame is sent at once. |
| `bool begin()` | Switches Vext on where the board has it, resets and clears the display (about 0.1 s, plus 100 ms on Heltec boards). Call once from `setup()`. False if there is no display; every other call then does nothing. |
| `bool ready() const` | `begin()` succeeded. |
| `void setDuckId(const std::string& id)`<br>`void setDuckId(const std::vector<uint8_t>& id)` | The ID on the first line. The vector form takes `Duck::getDuckId()`. |
| `void setDuckType(DuckType type)` | The type next to the ID ("Mama", "Papa", ...). |
| `template <typename DuckT> void setDuck(DuckT& duck)` | `setDuckId(duck.getDuckId())` and `setDuckType(duck.getType())`. |
| `void setStatus(const std::string& status)` | The second line. Empty shows the CDP version. Setting the same text again does not redraw. |
| `void showReceived(const CdpPacket& packet)` | Shows a packet the duck heard. Call it from the `onReceiveDuckData()` callback. |
| `void showSent(uint8_t topic, const std::string& payload, const Duid& destination = PAPADUCK_DUID)`<br>`void showSent(uint8_t topic, const uint8_t* data, size_t length, const Duid& destination = PAPADUCK_DUID)` | Shows a packet the duck sent. Call it after `sendData()` returned `DUCK_ERR_NONE`. |
| `void showLogo(uint32_t durationMs = 2000)` | Shows the ClusterDuck logo for `durationMs` from when it is drawn, then the status screen. Changes made meanwhile appear afterwards. |
| `void showLogo(const uint8_t* xbm, uint16_t width, uint16_t height, uint32_t durationMs = 2000)` | The same with your own XBM image, centred. The data must stay valid while it shows. |
| `void update()` | Call on every `loop()` pass. Draws a new frame when something changed and the refresh interval has passed, then sends `DUCKDISPLAY_TILE_ROWS_PER_UPDATE` rows of tiles per call. |
| `void refresh()` | Draws and sends the current screen at once (about 25 ms on hardware I2C, 0.1 s on software I2C). For `setup()`. |
| `void sleep()` | Display off (controller power save; the picture is kept). Packets are still recorded. Call it before deep sleep. |
| `void wake()` | Display on, and redraws. |
| `bool asleep() const` | `sleep()` was called and `wake()` was not. |
| `void setRefreshInterval(uint32_t ms)` | Shortest time between two frames (default `DUCKDISPLAY_REFRESH_MS`, 250 ms). |
| `const DuckDisplay::PacketInfo& lastPacket() const` | What the screen shows about the last packet (below). |
| `uint32_t receivedCount() const`, `uint32_t sentCount() const` | Packets passed to `showReceived()` and `showSent()` since start-up. |
| `U8G2* oled()` | The U8g2 driver, for settings such as `setContrast()`, or `nullptr` without a display. Drawing on it directly is overwritten by the next frame. |

`DuckDisplay::PacketInfo`:

| Field | Meaning |
|-------|---------|
| `bool valid` | false until the first `showReceived()` or `showSent()` |
| `bool received` | true: heard by the duck; false: sent by it |
| `uint8_t topic`, `std::string topicName` | CDP topic and its name ("sensor", "gps", or "0x42" when CDP doesn't name it) |
| `std::string peer` | heard: the origin (`sduid`); sent: the destination. "papa" for `PAPADUCK_DUID`, "all" for `BROADCAST_DUID` |
| `std::string preview` | the payload as text, at most `DUCKDISPLAY_PREVIEW_LEN` (40) characters; non-printable bytes as two hex digits |
| `uint8_t hopCount` | heard: how often it was relayed before it got here |
| `bool hasSignal`, `float rssiDbm`, `float snrDb` | heard: RSSI in dBm and SNR in dB from the radio. CDP stores them scaled to 0..1 in `CdpPacket::receivedSignal`; these are converted back. |
| `uint32_t number` | heard: `receivedCount()`; sent: `sentCount()` at the time |
| `unsigned long atMs` | `millis()` when it was reported |

Helpers in namespace `duckdisplay`: `std::string topicName(uint8_t topic)`, `const char* duckTypeName(DuckType type)`, `std::string duidLabel(const Duid& duid)`, and the logo `DUCK_LOGO_XBM` (`DUCK_LOGO_WIDTH` x `DUCK_LOGO_HEIGHT`, 128x64).

Constants: `DuckDisplay::DEFAULT_REFRESH_MS` (250), `DuckDisplay::DEFAULT_LOGO_MS` (2000), `DUCKDISPLAY_VERSION` ("1.1.0").

### The status screen

On a 128x64 display (21 characters by 6 lines, font `u8g2_font_6x10_tr`):

```
MAMA0001         Mama     ID and type
Running                   setStatus()
---------------------
RX sensor          #12    heard (RX) or sent (TX), topic, packet number
from DUCK0002             origin; "to papa" for a packet the duck sent
{"count":12,"uptimeS      payload preview
-87dBm 9.5dB     hop 1    heard only: RSSI, SNR, hop count
```

Before the first packet, the third line reads "No packets yet". Smaller displays show the first lines that fit: on a 64x32 display, the ID, the status and the RX/TX line. Text that doesn't fit is cut; the right-hand items are left out when they don't fit.

### Timing

- A frame is drawn only when something on it changed, and at most once per refresh interval (250 ms). Packets that arrive faster are combined.
- A 128x64 frame is 8 rows of tiles, 128 bytes each (140 with the commands that address them). `update()` sends one row per call. By bus arithmetic that is roughly 3 ms on hardware I2C at 400 kHz, and in the order of 15 ms on U8g2's software I2C; these are estimates, not measurements. The frame is out after 8 `loop()` passes.
- `begin()` and `refresh()` block, so call them from `setup()`.

### Deep sleep

Call `display.sleep()` before `esp_deep_sleep_start()`. With QuackPack-DuckSleep, register it as a power gate:

```cpp
duckSleep::addPowerGate([](bool on) {
    if (on) display.wake(); else display.sleep();
});
```

Waking from deep sleep is a reboot: `setup()` calls `begin()` again.

### Compile-time options

Set these with `-D` in `build_flags`.

| Define | Default | Meaning |
|--------|---------|---------|
| `DUCKDISPLAY_OLED_CLASS` | see [Wiring](#wiring-and-board-notes) | U8g2 class of the board's OLED for the default constructor. Must be an `_SW_I2C` or `_HW_I2C` class; for others, use `DuckDisplay(U8G2&)`. |
| `DUCKDISPLAY_OLED_ROTATION` | `CDPCFG_PIN_OLED_ROTATION`, else `U8G2_R0` | Rotation of the board's OLED (`U8G2_R2` turns it 180 degrees). |
| `DUCKDISPLAY_OLED_RESET` | `CDPCFG_PIN_OLED_RESET`, else none | Reset pin of the board's OLED. |
| `DUCKDISPLAY_PIN_VEXT` | `CDPCFG_PIN_VEXT`; 36 on Heltec V3; else -1 | Pin `begin()` drives low to power the OLED. -1: none. |
| `DUCKDISPLAY_VEXT_SETTLE_MS` | 100 | Wait after switching Vext on. |
| `DUCKDISPLAY_REFRESH_MS` | 250 | Shortest time between two frames. |
| `DUCKDISPLAY_TILE_ROWS_PER_UPDATE` | 1 | Rows of tiles `update()` sends per call. |
| `DUCKDISPLAY_PREVIEW_LEN` | 40 | Characters of payload kept in `PacketInfo::preview`. |

CDP's own options still apply: `ENABLE_DISPLAY` (T-Beam OLED pins) and `CDPCFG_OLED_64x32` (64x32 class).

## Migrating from 1.0.0

1.0.0 was written against pre-5.5 CDP and depended on `QuackPack.h` from QuackPack-Base, which that repository does not have.

- `AdafruitDisplay` and `U8G2Display<Driver>` are replaced by one class, `DuckDisplay`, without templates. SSD1306 modules work through U8g2; the Adafruit SSD1306, GFX and BusIO libraries are no longer dependencies.
- `U8G2Display<Driver> disp(w, h, driver, rst)` becomes `DuckDisplay display(driver);` the size comes from the driver.
- `launch()` becomes `begin()`, which returns `bool` and no longer calls `Serial.begin()`.
- `showDefaultScreen()` is gone: `update()` draws the status screen. Use `setDuck()` and `setStatus()` for what it shows.
- `printPacketHeader(packet)` becomes `showReceived(packet)`.
- `setLogo(vector)` plus `showLogo(type, duration)` (which called `delay()`) becomes `showLogo(durationMs)` for the built-in logo, or `showLogo(xbm, width, height, durationMs)`. Only XBM images are accepted; convert Adafruit bitmaps to XBM.
- `sleep()` and `wake()` keep their names.
- `clear()` and the public `display` member are gone; `oled()` returns the U8g2 driver.
- `duckTypeToString()` becomes `duckdisplay::duckTypeName()`; `replaceNonPrintableWithHex()` is CDP's `duckutils::toString()`.
- No dependency on QuackPack-Base or on a CDP fork in `library.json`.

## Building the examples

```bash
pio run                                   # every example environment
pio run -e heltec_wifi_lora_32_V3 -t upload
pio device monitor
```

| Environment | Example | Board |
|-------------|---------|-------|
| `lilygo_t_beam_sx1262` | [MamaDuck](examples/MamaDuck) | `ttgo-t-beam`, `-D ENABLE_DISPLAY` |
| `heltec_wifi_lora_32_V3` | [MamaDuck](examples/MamaDuck) | `heltec_wifi_lora_32_V3` |
| `heltec_wifi_lora_32_V2` | [MamaDuck](examples/MamaDuck) | `heltec_wifi_lora_32_V2` |
| `custom_driver_lilygo_t_beam_sx1262` | [CustomDriver](examples/CustomDriver) | `ttgo-t-beam`, external SH1106 |

`platformio.ini` uses the same platform as the DMS builder (espressif32 7.0.1), CDP's build flags and `no_ota.csv` partition table, and CDP 5.5 pinned by commit. It uses this checkout of the QuackPack (`symlink://.`). GitHub Actions builds every environment on every push and pull request.

## Compatibility

- **ClusterDuck Protocol**: 5.x (built against Project-Owl CDP 5.5, c293c742)
- **U8g2**: 2.36.18 or later
- **PlatformIO**: 6.1+
- **Platform**: espressif32 7.0.1 (Arduino core 2.0.17)
- **Development environment**: PlatformIO only (Arduino IDE not supported)

## Contributing

Contributions are welcome. Open an [issue](https://github.com/Project-Owl/QuackPack-DuckDisplay/issues) or a pull request; `pio run` must pass for every environment.

## Changelog

### 1.1.0

- One `DuckDisplay` class over any U8g2 driver; the Adafruit backend and the `U8G2Display` template are gone.
- Status screen with the duck's ID, type and status, and the last packet heard or sent (topic, origin or destination, payload preview, RSSI, SNR, hop count).
- Non-blocking `update()`: frames are drawn only on change, at most every 250 ms, and sent one row of tiles per call.
- Default display from the CDP board header; hardware I2C on boards that share GPIO21/22 with a PMU; Vext switched on for Heltec boards.
- Built against CDP 5.5 (`onReceiveDuckData()`, `CdpPacket::receivedSignal`, `Duid`, `duckutils`).
- No CDP or QuackPack-Base dependency in `library.json`; `library.properties` added.
- Examples and CI build for the LilyGO T-Beam and Heltec V2 and V3.

### 1.0.0

- Initial release

## License

This project is licensed under the Apache 2.0 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built on the [ClusterDuck Protocol](https://github.com/ClusterDuck-Protocol/ClusterDuck-Protocol)
- Drawing by [U8g2](https://github.com/olikraus/u8g2)
- Created by [OWL Integrations](https://www.owlintegrations.com/)

## Resources

- [OWL Integrations](https://www.owlintegrations.com/)
- [ClusterDuck Protocol](https://github.com/ClusterDuck-Protocol/ClusterDuck-Protocol)
- [U8g2 setup reference](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp)
- [Discord Community](https://discord.gg/clusterduck)

## Support

For questions and support:
- Join the [ClusterDuck Protocol Discord](https://discord.gg/clusterduck)
- Open an [Issue](https://github.com/Project-Owl/QuackPack-DuckDisplay/issues)

## Authors

- **Brenton Poke** - *Initial work* - [@BrentonPoke](https://github.com/brentonpoke)
