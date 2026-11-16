# MamaDuck

A MamaDuck (device ID `MAMA0001`) that shows what it does on the board's OLED.

- `setup()` starts the display first and shows the ClusterDuck logo with `refresh()`, so the logo is up while the radio starts. Then it starts the duck and sets the status to "Running".
- Every packet the duck hears goes to `display.showReceived()` from the `onReceiveDuckData()` callback, and is logged with its RSSI and SNR.
- Every 30 s it sends a text message on `topics::sensor` and, once it is queued, shows it with `display.showSent()`.
- `loop()` calls `display.update()` on every pass. It sends at most one row of tiles per call, so the duck keeps relaying while a frame goes out.

The display is the board's OLED from the CDP board header. On a LilyGO T-Beam the OLED is optional: build with `-D ENABLE_DISPLAY` (the `lilygo_t_beam_sx1262` environment does). On a board without one, `begin()` returns false and the duck runs the same.

## Build and flash

From the repository root:

```bash
pio run -e heltec_wifi_lora_32_V3 -t upload
pio device monitor
```

The environments `lilygo_t_beam_sx1262`, `heltec_wifi_lora_32_V3` and `heltec_wifi_lora_32_V2` build this sketch with espressif32 7.0.1 and Project-Owl CDP 5.5 pinned by commit. They use this checkout of the QuackPack (`symlink://.`).

## Using it in your own project

Copy `src/main.cpp`. In your own `platformio.ini`, pin `QuackPack-DuckDisplay` by commit next to your CDP pin. The main [README](../../README.md#installation) explains how.
