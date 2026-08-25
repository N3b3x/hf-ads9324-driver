---
layout: default
title: CMake
nav_order: 7
---

# CMake

Standalone:

```cmake
add_subdirectory(hf-ads9324-driver)
target_link_libraries(app PRIVATE hf::ads9324)
```

Host tests:

```bash
cmake -S . -B build -DHF_ADS9324_BUILD_HOST_TESTS=ON
cmake --build build
ctest --test-dir build
```

hf-core: `HF_CORE_ENABLE_ADS9324` includes `Ads9324Handler.cpp` and the driver include path. Default in hf-core **and** product `pw_hal_core_features.cmake` is **OFF**. ESP32 hf-core examples turn the flag ON so the handler and `ads9324_handler_test` compile without putting the chip in the product image.
