---
layout: default
title: Quick start
nav_order: 3
---

# Quick start

1. Implement `ads9324::SpiInterface` (full-duplex, CS around each `transfer`).
2. Implement `ads9324::HostInterface` (CONVST output, optional DRDY, delays).
3. Call `EnsureInitialized()` — waits 30 ms, software-resets, programs **GEN_CFG3 = 0x0032** (1-lane, 16-bit, data on SDOUT), reads **DEVICE_ID = 0x0004**.
4. `ConfigureChannel` / `ConfigureAllChannels` for PGA range and input type.
5. `ReadSnapshot()` pulses CONVST, waits DRDY or 10 µs, clocks 16×16 bits on SDOUT.

Do not clock conversion data in the default 8-lane D[7:0] mode unless the host has a parallel capture engine. This driver always programs 1-lane SDOUT for MCU/ESP32 bring-up.
