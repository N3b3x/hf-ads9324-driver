---
layout: default
title: Configuration
nav_order: 13
---

# Configuration

Compile-time knobs in `ads9324_config.hpp` (override with `-DCONFIG_ADS9324_*`):

| Macro | Effect |
|-------|--------|
| `CONFIG_ADS9324_RANGE_12V5` / `_10V` / `_2V5` | Default PGA full-scale |
| `CONFIG_ADS9324_SINGLE_ENDED` | Default input type |
| `CONFIG_ADS9324_PGA_WIDE` | Default analog LPF |
| `CONFIG_ADS9324_OFFSET_BINARY` | Wire format |
| `CONFIG_ADS9324_SKIP_DEVICE_ID_CHECK` | Bring-up without ID match |
| `CONFIG_ADS9324_POWER_ON_DELAY_MS` | Default 30 |
| `CONFIG_ADS9324_DRDY_TIMEOUT_US` | Default 1000 |
| `CONFIG_ADS9324_CONV_FALLBACK_US` | Used when DRDY is not wired |
