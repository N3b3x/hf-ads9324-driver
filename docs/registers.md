---
layout: default
title: Registers
nav_order: 5
---

# Registers and SPI frames

Configuration SPI is **24 bits**: `SDI[23:16] = address`, `SDI[15:0] = data`. CS rising edge commits the write (SBASB22 §7.5.1).

Three banks, selected by writing `BANK_SEL` (0x02):

| Value | Bank |
|-------|------|
| `0x0001` | Common |
| `0x0002` | AIN1–AIN8 |
| `0x0004` | AIN9–AIN16 |

Register read (Table 7-17): select bank, write `GEN_CFG1` with `REG_RD_ADD` in bits 15:8 and `REG_RD_EN=1`, then a dummy `0x000000` frame. Register data returns in `SDOUT[23:8]`.

Default bring-up (Table 7-18): `GEN_CFG3 = 0x0032` → 1 lane, 16-bit length, `ADC_DATA_SDOUT_EN=1`.
