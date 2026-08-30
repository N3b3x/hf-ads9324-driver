---
layout: default
title: Hardware setup
nav_order: 8
---

# Hardware setup

This driver uses the **1-lane SDOUT** digital path (GEN_CFG3 = `0x0032` at
bring-up). Parallel `D[7:0]` is not captured — leave those pins unconnected
or parked per SBASB22 if the board does not route them.

## Minimum wiring

| ADS9324 | Host | Notes |
|---------|------|--------|
| SDI | MOSI | Config + dummy clocks during conversion readout |
| SDOUT | MISO | Conversion data in 1-lane mode |
| SCLK | SCLK | Mode 0 (CPOL=0, CPHA=0). 8 MHz is the ESP32-C6 example default |
| CS | CS | Active low. One CS per `transfer()` call; do not deassert mid-frame |
| CONVST | GPIO out | Active-high pulse; **falling** edge starts conversion (≥ 50 ns high) |
| DRDY/ALARM | GPIO in | Optional. Default mux is conversion-complete. If NC, driver waits `CONV_FALLBACK_US` |
| IOVDD / AVDD / REFIO | Supplies | Bypass per SBASB22 recommended layout |

ESP32-C6 example defaults (edit `examples/esp32/main/esp32_ads9324_test_config.hpp`):

| Function | GPIO |
|----------|------|
| SDOUT (MISO) | 5 |
| SDI (MOSI) | 6 |
| SCLK | 4 |
| CS | 10 |
| CONVST | 3 |
| DRDY | 11 |

Map host GPIO “active” to CONVST high.

## Timing (SBASB22)

- Wait **30 ms** after analog/digital rails before the first SPI frame (Table 7-18). The driver does this in `EnsureInitialized`.
- After `GEN_CFG1.SW_RST`, wait `RESET_DELAY_MS` (default 1 ms) then reprogram GEN_CFG3.
- Config frames are **24 bits** (8-bit address + 16-bit data), latched on SCLK rising, decoded on CS rising (§7.5).
- Conversion readout is one CS-low burst: **32 bytes** for 16 channels × 16-bit, or 48 bytes in 24-bit `DOUT_LENGTH`.
