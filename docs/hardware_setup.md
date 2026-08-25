---
layout: default
title: Hardware setup
nav_order: 8
---

# Hardware setup

Minimum wires for the 1-lane SDOUT path this driver uses:

| ADS9324 | Host |
|---------|------|
| SDI | MOSI |
| SDOUT | MISO |
| SCLK | SCLK |
| CS | CS (active low) |
| CONVST | GPIO out |
| DRDY/ALARM | GPIO in (optional) |
| IOVDD / AVDD / REFIO | Per SBASB22 recommended bypass |

Power-on: wait 30 ms before SPI (Table 7-18). CONVST pulse high ≥ 50 ns, conversion on falling edge. Default `GEN_CFG3 = 0x0032` puts conversion data on SDOUT so a normal SPI controller can read all 16 channels after DRDY.
