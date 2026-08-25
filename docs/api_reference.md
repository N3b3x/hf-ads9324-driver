---
layout: default
title: API reference
nav_order: 4
---

# API reference

Class: `ads9324::ADS9324<SpiType, HostType>`

| Method | Purpose |
|--------|---------|
| `EnsureInitialized(force)` | Rail delay, SW reset, 1-lane SDOUT, DEVICE_ID |
| `SoftwareReset()` | GEN_CFG1.SW_RST |
| `ReadDeviceId()` | Common 0x21, expect `0x0004` |
| `ConfigureChannel(ch, cfg)` | PGA nibble + bandwidth for channel 0–15 |
| `ConfigureAllChannels(cfg)` | All 16 channels |
| `SetChannelOffset(ch, ofs10)` | OFS_AINx, 10-bit two's complement |
| `SetChannelGain(ch, gan14)` | GAN_AINx, 14-bit |
| `SetDataFormat` | Two's complement or offset binary |
| `SetDigitalInterface` | Lane count / 16 vs 24 bit / SDOUT vs D7 |
| `SetChannelCountMode` | 16/8/4/2 channel readout (Table 7-12) |
| `SetPowerDown` | PDN_CTL.DEVICE_PDN |
| `SetAlarmPin` | DRDY vs DWC vs CAL_DONE |
| `ProgramWindowComparator` | DWC_TH + DWC_EN |
| `ReadSnapshot` | CONVST + burst |
| `ReadChannel` | Snapshot then pick one channel (16-ch mode) |
| `CountToVoltage` | Uses that channel's programmed FS |

Types live in `ads9324_types.hpp`. Frame packing lives in `ads9324_registers.hpp`.
