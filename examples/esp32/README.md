# ESP32-C6 ADS9324 examples

Target is **esp32c6**. The `scripts/` tree is the shared [hf-espidf-project-tools](https://github.com/N3b3x/hf-espidf-project-tools) submodule.

## Apps

| App | Source | Notes |
|-----|--------|-------|
| `driver_integration_test` | `driver_integration_test.cpp` | DEVICE_ID, PGA, snapshot, cal, power-down |
| `basic_snapshot` | `basic_snapshot_example.cpp` | Live 16-ch print loop |
| `pga_offset` | `pga_offset_example.cpp` | Mixed ranges + OFS/GAN |

```bash
cd examples/esp32
idf.py set-target esp32c6
./scripts/build_app.sh list
./scripts/build_app.sh driver_integration_test Debug
./scripts/flash_app.sh driver_integration_test Debug
```

Edit `main/esp32_ads9324_test_config.hpp` for your wiring. CONVST is required. DRDY can be disabled (`USE_DRDY = false`) to use a timed wait.

`TestFramework.h` records pass/fail and toggles a progress GPIO (GPIO14 when present).
