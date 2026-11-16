# MamaDuck

A MamaDuck (device ID `MAMA0001`) that shows what it does on the board's OLED. It uses every part of the DuckDisplay API on ClusterDuck Protocol 5.5.

- `setup()` starts the display and shows the ClusterDuck logo with `refresh()`, so the logo is up while the radio starts. Then it starts the duck and sets the status to "Running".
- Every packet the duck hears goes to `display.showReceived()` from the `onReceiveDuckData()` callback, and is logged with its number, RSSI and SNR from `display.lastPacket()`.
- Every 30 s it sends a text message on `topics::sensor` and, once it is queued, shows it with `display.showSent()`. A send error goes to the status line.
- `loop()` calls `display.update()` on every pass. It sends at most one row of tiles per call, so the duck keeps relaying while a frame goes out.

## LilyGO T-Beam: the display and the PMU share the I2C bus

On a T-Beam v1.2 the AXP2101 PMU and the OLED both sit on GPIO21/22. The sketch starts `Wire` for the PMU first (`Wire.begin(21, 22)` and XPowersLib), then `display.begin()`. On this board DuckDisplay uses hardware I2C, so it joins the `Wire` bus the sketch started instead of bit-banging the pins. Every 10 s the sketch reads the battery from the PMU and shows it on the status line:

```
MAMA0001         Mama
Running  3.98V 87%
```

The T-Beam's OLED is optional: build with `-D ENABLE_DISPLAY` (the `lilygo_t_beam_sx1262` environments do). On a T-Beam v1.0 or v1.1 (AXP192) the battery reading is left out. On Heltec boards there is no PMU and the status line shows only the state.

## Optional deep sleep

Build with `-D ENABLE_DEEP_SLEEP=1`, as the `lilygo_t_beam_sx1262_deep_sleep` environment does. The duck then stays awake for 60 s after each boot, waits until `duck.txIdle()` (at most 10 s), and then:

1. `duck.prepareForSleep()` puts the radio to sleep;
2. `display.sleep()` turns the display off;
3. `esp_deep_sleep_start()` sleeps for 300 s.

Waking up is a reboot; the logo is only shown after power-up. A sleeping MamaDuck relays nothing, so use this only for a duck that no other duck needs as a relay.

## Build and flash

From the repository root:

```bash
pio run -e lilygo_t_beam_sx1262 -t upload
pio device monitor
```

| Environment | Board | Notes |
|-------------|-------|-------|
| `lilygo_t_beam_sx1262` | `ttgo-t-beam` | `-D ENABLE_DISPLAY`, battery from the AXP2101 |
| `lilygo_t_beam_sx1262_deep_sleep` | `ttgo-t-beam` | the same, with `-D ENABLE_DEEP_SLEEP=1` |
| `heltec_wifi_lora_32_V3` | `heltec_wifi_lora_32_V3` | |
| `heltec_wifi_lora_32_V2` | `heltec_wifi_lora_32_V2` | |

They build with espressif32 7.0.1 and Project-Owl CDP 5.5 pinned by commit, and use this checkout of the QuackPack (`symlink://.`).

## Using it in your own project

Copy `src/main.cpp`. In your own `platformio.ini`, pin `QuackPack-DuckDisplay` by commit next to your CDP pin, and add `lewisxhe/XPowersLib@^0.3.0` for the T-Beam battery reading. The main [README](../../README.md#installation) explains how.
