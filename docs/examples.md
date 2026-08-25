---
layout: default
title: Examples
nav_order: 12
---

# Examples

ESP32-C6 apps under `examples/esp32/`:

- `driver_integration_test` — full TestFramework suite
- `basic_snapshot` — print loop
- `pga_offset` — mixed PGA + calibration

Host (no IDF): `tests/host/test_ads9324_protocol.cpp`.

hf-core handler test (in the parent hf-core tree, ESP32-S3 examples, flag ON only there):
`examples/esp32/main/handler_tests/ads9324_handler_comprehensive_test.cpp`
(`APP_TYPE=ads9324_handler_test`).
