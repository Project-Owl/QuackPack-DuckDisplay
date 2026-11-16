# CustomDriver

A MamaDuck (device ID `MAMAOLED`) with an external OLED that the sketch sets up itself: a 1.3" SH1106 module on the ESP32's Wire pins (SDA GPIO21, SCL GPIO22), turned 180 degrees.

```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
DuckDisplay display(oled);
```

`DuckDisplay(U8G2&)` takes any U8g2 driver; the [U8g2 setup reference](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp) lists them. Pick a full-buffer class (`_F_`) so `update()` can send a frame in parts, and hardware I2C (`_HW_I2C`) when other devices share the bus.

## Build and flash

From the repository root:

```bash
pio run -e custom_driver_lilygo_t_beam_sx1262 -t upload
pio device monitor
```
