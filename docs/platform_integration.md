---
layout: default
title: Platform integration
nav_order: 11
---

# Platform integration

The driver has no OS or MCU headers. Bind two CRTP types, then construct
`ads9324::ADS9324<Spi, Host>`.

## `SpiInterface<Derived>`

Implement:

```cpp
void transfer(const uint8_t* tx, uint8_t* rx, size_t len);
```

Contract:

- Assert CS for the **entire** `len`, then deassert. Config frames are 3 bytes.
  Conversion bursts are `N * 2` bytes (16-bit) or `N * 3` (24-bit).
- `tx == nullptr` means clock zeros (conversion readout).
- `rx == nullptr` means discard MISO (config writes).
- SPI mode 0, MSB first. 8 MHz is a conservative start vs the 1 MSPS analog path.

## `HostInterface<Derived>`

| Method | Role |
|--------|------|
| `ConvstWrite(bool high)` | CONVST pin. High then low; conversion on falling edge |
| `HasDrdy() const` | `false` → timed wait (`ADS9324_CFG::CONV_FALLBACK_US`) |
| `DrdyRead()` | Sample DRDY/ALARM (conversion-complete in default mux) |
| `DelayUs` / `DelayMs` | POR (30 ms), reset settle, DRDY poll step |

## Reference implementation

ESP32-C6: `examples/esp32/main/esp32_ads9324_bus.hpp`.

A consuming platform (handler, HAL, Linux spidev, …) implements the same two
CRTP contracts. This driver does not include or name those layers.

## Channel numbering

API channel 0 = hardware AIN1, channel 15 = AIN16.
Register names in SBASB22 stay 1-based.
