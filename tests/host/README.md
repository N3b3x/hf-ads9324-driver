# Host protocol tests

No silicon. Mock SPI register file + CONVST counter.

```bash
cmake -S . -B build -DHF_ADS9324_BUILD_HOST_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
