---
layout: default
title: "HardFOC ADS9324 Driver"
description: "Portable C++20 driver for the TI ADS9324 16-channel 16-bit simultaneous-sampling SAR ADC with integrated PGA, offset/gain, and 1-lane SDOUT"
nav_order: 1
permalink: /
---

# HF-ADS9324 Driver

**Portable C++20 driver for the Texas Instruments ADS9324 — 16-channel, 16-bit, 1 MSPS simultaneous-sampling SAR ADC with integrated analog front-end (PGA, differential / single-ended, offset and gain calibration).**

This repository owns the silicon protocol only. Hosts implement `SpiInterface` and `HostInterface` (see `examples/esp32/` for one CRTP backend).

## Overview

The ADS9324 (TI SBASB22, December 2025) is a 16-channel data-acquisition SoC: each channel has a clamp, PGA, and SAR. CONVST falling edge samples all enabled channels together. The default digital path in this driver is **1-lane 16-bit conversion data on SDOUT** plus 24-bit configuration SPI on the same SDI/SCLK/CS pins (Table 7-18), so a normal SPI controller can bring the part up without a multi-lane capture engine.

Datasheet (PDF + extracted text) lives in [`docs/datasheet/`](docs/datasheet/).

## Features

- 16 simultaneous 16-bit channels (hardware AIN1–AIN16 → API channels 0–15)
- Integrated PGA: differential, single-ended, open-wire-safe; ±2.5 / ±5 / ±6.25 / ±10 / ±12.5 V
- Per-channel analog LPF (low ~25.5 kHz or wide 280–350 kHz)
- 10-bit offset and 14-bit gain calibration registers
- Digital window comparator
- CONVST + optional DRDY host pins
- CRTP `SpiInterface` + `HostInterface` (zero virtual overhead)
- ESP32-C6 examples + `TestFramework.h` integration suite
- Host protocol unit tests (no silicon)

## Quick start

```cpp
#include "ads9324.hpp"

class MySpi : public ads9324::SpiInterface<MySpi> {
 public:
  void transfer(const uint8_t* tx, uint8_t* rx, size_t len) { /* CS-scoped SPI */ }
};
class MyHost : public ads9324::HostInterface<MyHost> {
 public:
  void ConvstWrite(bool high);
  bool DrdyRead();
  bool HasDrdy() const { return true; }
  void DelayUs(uint32_t us);
  void DelayMs(uint32_t ms);
};

MySpi spi;
MyHost host;
ads9324::ADS9324<MySpi, MyHost> adc(spi, host);
adc.EnsureInitialized();  // 30 ms rail settle, SW reset, GEN_CFG3=0x0032, DEVICE_ID

ads9324::ChannelConfig cfg;
cfg.type = ads9324::InputType::Differential;
cfg.range = ads9324::InputRange::PlusMinus10V;
adc.ConfigureChannel(0, cfg);

auto snap = adc.ReadSnapshot();
```

## ESP32-C6

```bash
cd examples/esp32
idf.py set-target esp32c6
./scripts/build_app.sh driver_integration_test Debug
./scripts/flash_app.sh driver_integration_test Debug
```

Default pins (edit `main/esp32_ads9324_test_config.hpp`):

| Function | GPIO |
|----------|------|
| SDOUT (MISO) | 5 |
| SDI (MOSI) | 6 |
| SCLK | 4 |
| CS | 10 |
| CONVST | 3 |
| DRDY | 11 |

## License

GPL-3.0 — see [LICENSE](LICENSE).
