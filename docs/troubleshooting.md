---
layout: default
title: Troubleshooting
nav_order: 10
---

# Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| `EnsureInitialized` fails DEVICE_ID | SPI mode not 0, CS polarity, 24-bit frame packing, analog rail not up |
| DEVICE_ID is 0 | Read protocol skipped dummy frame; SDOUT not connected |
| Snapshot all zeros / timeout | CONVST not pulsing, DRDY polarity, still in 8-lane D[7:0] mode |
| PGA write does not stick | Wrong register bank (AIN1–8 vs AIN9–16) |
| Voltage scale wrong | Channel `InputRange` not programmed; two's vs offset-binary |

Host protocol tests (no silicon):

```bash
cmake -S . -B build -DHF_ADS9324_BUILD_HOST_TESTS=ON && cmake --build build && ctest --test-dir build
```
