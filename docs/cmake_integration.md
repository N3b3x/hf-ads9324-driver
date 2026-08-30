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
