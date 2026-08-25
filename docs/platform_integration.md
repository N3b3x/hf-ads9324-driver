---
layout: default
title: Platform integration
nav_order: 11
---

# Platform integration

Implement two CRTP types:

1. `SpiInterface` — `transfer(tx, rx, len)` with CS around the whole call. Config frames are 3 bytes. Conversion bursts are 32 bytes (16-bit × 16 ch) or 48 bytes (24-bit mode).
2. `HostInterface` — CONVST write, optional DRDY read, `DelayUs` / `DelayMs`.

ESP32-C6 reference: `examples/esp32/main/esp32_ads9324_bus.hpp`.

hf-core: `Ads9324Handler` wraps `BaseSpi` + `BaseGpio`. CONVST is active-high (`SetActive` = high).
