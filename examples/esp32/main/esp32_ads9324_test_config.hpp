/**
 * @file esp32_ads9324_test_config.hpp
 * @brief Hardware configuration for ADS9324 bring-up on ESP32-C6
 * @ingroup ads9324_examples_support
 *
 * Default pins target ESP32-C6-DevKitC / DevKitM SPI2 (FSPI) plus two GPIOs
 * for CONVST and DRDY. Edit this file to match your wiring.
 */
#pragma once

#include <cstdint>

#ifndef ESP32_ADS9324_ENABLE_DETAILED_SPI_LOGGING
#define ESP32_ADS9324_ENABLE_DETAILED_SPI_LOGGING 1
#endif

namespace ADS9324_TestConfig {

struct SPIPins {
    static constexpr uint8_t MISO = 5;   ///< SDOUT
    static constexpr uint8_t MOSI = 6;   ///< SDI
    static constexpr uint8_t SCLK = 4;   ///< SCLK
    static constexpr uint8_t CS   = 10;  ///< CS (active low)
};

struct GpioPins {
    static constexpr uint8_t CONVST = 3;   ///< Conversion start (active high pulse)
    static constexpr uint8_t DRDY   = 11;  ///< Data ready (active high). Set USE_DRDY=false if NC.
    static constexpr bool USE_DRDY  = true;
};

struct SPIParams {
    static constexpr uint32_t FREQUENCY = 8000000;  ///< 8 MHz (conservative vs 1 MSPS path)
    static constexpr uint8_t MODE = 0;              ///< CPOL=0, CPHA=0
    static constexpr uint8_t QUEUE_SIZE = 1;
    static constexpr uint8_t CS_ENA_PRETRANS = 2;
    static constexpr uint8_t CS_ENA_POSTTRANS = 2;
    static constexpr uint8_t SPI_HOST_ID = 1;       ///< SPI2_HOST on ESP32-C6
};

struct ADCSpecs {
    static constexpr uint8_t  NUM_CHANNELS    = 16;
    static constexpr uint8_t  RESOLUTION_BITS = 16;
    static constexpr uint16_t DEVICE_ID       = 0x0004;
};

}  // namespace ADS9324_TestConfig
