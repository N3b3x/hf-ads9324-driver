---
layout: default
title: PGA and calibration
nav_order: 6
---

# PGA, offset, and gain

Each channel's `PGA_CONFIG` nibble (Table 7-1 / register 0x08–0x0B):

- `INPUT_RANGE[2:0]`: 0=±5 V, 2=±2.5 V, 3=±6.25 V, 4=±10 V, 5=±12.5 V
- `CM_RANGE[2:0]`: 0=differential, 5=single-ended, 6=open-wire-safe
- `CME_CORR_EN`: common-mode error correction

`PGA_BW_SEL` (0x0C): 2 bits per channel, 0=low bandwidth (~25.5 kHz), 1=wide.

Offset (Equation 1): 10-bit two's-complement `OFS_AINx` added as `(code×4 + OFS)/4`.

Gain (Equation 2): 14-bit `GAN_AINx`, correction = GAN / 65536.
