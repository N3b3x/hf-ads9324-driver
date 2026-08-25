---
layout: default
title: Installation
nav_order: 9
---

# Installation

```bash
git clone --recurse-submodules https://github.com/N3b3x/hf-ads9324-driver.git
```

CMake consumer:

```cmake
add_subdirectory(hf-ads9324-driver)
target_link_libraries(your_app PRIVATE hf::ads9324)
```

ESP-IDF: use `examples/esp32` and `idf.py set-target esp32c6`.

hf-core: nested submodule `hf-core-drivers/external/hf-ads9324-driver` plus `HF_CORE_ENABLE_ADS9324` (OFF by default; ON in ESP32 examples only).
