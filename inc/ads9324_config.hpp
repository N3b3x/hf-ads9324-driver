/**
 * @file ads9324_config.hpp
 * @brief Compile-time defaults for the ADS9324 driver
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_config
 *
 * @details
 * Override any knob with a `CONFIG_ADS9324_*` preprocessor define (ESP-IDF
 * Kconfig, CMake `target_compile_definitions`, or a board header). Unset
 * macros keep SBASB22 bring-up defaults: ±5 V differential, 1-lane 16-bit
 * data on SDOUT, 30 ms power-on delay.
 */
#pragma once

#include "ads9324_types.hpp"

namespace ADS9324_CFG {

/**
 * @defgroup ads9324_config Compile-Time Configuration
 * @ingroup ads9324_driver
 * @brief Build-time PGA, coding, and timing defaults.
 * @{
 */

// ---- Default analog range --------------------------------------------------
#ifdef CONFIG_ADS9324_RANGE_12V5
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus12V5;
#elif defined(CONFIG_ADS9324_RANGE_10V)
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus10V;
#elif defined(CONFIG_ADS9324_RANGE_2V5)
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus2V5;
#else
/** @brief Default PGA full-scale applied to every channel at construction. */
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus5V;
#endif

#ifdef CONFIG_ADS9324_SINGLE_ENDED
inline constexpr ads9324::InputType DEFAULT_INPUT_TYPE = ads9324::InputType::SingleEnded;
#else
/** @brief Default PGA topology (differential unless CONFIG_ADS9324_SINGLE_ENDED). */
inline constexpr ads9324::InputType DEFAULT_INPUT_TYPE = ads9324::InputType::Differential;
#endif

#ifdef CONFIG_ADS9324_PGA_WIDE
inline constexpr ads9324::PgaBandwidth DEFAULT_BANDWIDTH = ads9324::PgaBandwidth::Wide;
#else
/** @brief Default analog LPF (~25.5 kHz unless CONFIG_ADS9324_PGA_WIDE). */
inline constexpr ads9324::PgaBandwidth DEFAULT_BANDWIDTH = ads9324::PgaBandwidth::Low;
#endif

#ifdef CONFIG_ADS9324_OFFSET_BINARY
inline constexpr ads9324::DataFormat DEFAULT_FORMAT = ads9324::DataFormat::OffsetBinary;
#else
/** @brief Default conversion coding (two's-complement unless CONFIG_ADS9324_OFFSET_BINARY). */
inline constexpr ads9324::DataFormat DEFAULT_FORMAT = ads9324::DataFormat::TwosComplement;
#endif

#ifdef CONFIG_ADS9324_SKIP_DEVICE_ID_CHECK
inline constexpr bool REQUIRE_DEVICE_ID = false;
#else
/**
 * @brief When true, @ref ads9324::ADS9324::EnsureInitialized fails if DEVICE_ID
 *        is not @ref ads9324::kDeviceIdAds9324. Disable with
 *        CONFIG_ADS9324_SKIP_DEVICE_ID_CHECK for unpowered bench bring-up.
 */
inline constexpr bool REQUIRE_DEVICE_ID = true;
#endif

#ifdef CONFIG_ADS9324_POWER_ON_DELAY_MS
inline constexpr uint32_t POWER_ON_DELAY_MS = CONFIG_ADS9324_POWER_ON_DELAY_MS;
#else
/** @brief Wait after rails before SPI (SBASB22 Table 7-18). Override with CONFIG_ADS9324_POWER_ON_DELAY_MS. */
inline constexpr uint32_t POWER_ON_DELAY_MS = 30;
#endif

#ifdef CONFIG_ADS9324_RESET_DELAY_MS
inline constexpr uint32_t RESET_DELAY_MS = CONFIG_ADS9324_RESET_DELAY_MS;
#else
/** @brief Wait after GEN_CFG1.SW_RST before reprogramming GEN_CFG3. */
inline constexpr uint32_t RESET_DELAY_MS = 1;
#endif

#ifdef CONFIG_ADS9324_DRDY_TIMEOUT_US
inline constexpr uint32_t DRDY_TIMEOUT_US = CONFIG_ADS9324_DRDY_TIMEOUT_US;
#else
/** @brief Poll budget for DRDY after CONVST falling edge (1 µs steps). */
inline constexpr uint32_t DRDY_TIMEOUT_US = 1000;
#endif

#ifdef CONFIG_ADS9324_CONV_FALLBACK_US
inline constexpr uint32_t CONV_FALLBACK_US = CONFIG_ADS9324_CONV_FALLBACK_US;
#else
/** @brief Used when @ref ads9324::HostInterface::HasDrdy is false. */
inline constexpr uint32_t CONV_FALLBACK_US = 10;
#endif

/** @brief Default GEN_CFG3 lane mode — 1-lane SDOUT (supported capture path). */
inline constexpr ads9324::DoutLaneMode DEFAULT_LANES = ads9324::DoutLaneMode::Lane1;

/** @brief Default conversion word length. */
inline constexpr ads9324::DoutLength DEFAULT_LENGTH = ads9324::DoutLength::Bits16;

/**
 * @brief Default GEN_CFG3.ADC_DATA_SDOUT_EN.
 *
 * When true, conversion data appears on SDOUT so a normal SPI controller can
 * read all enabled channels after DRDY without a parallel D[7:0] capture engine.
 */
inline constexpr bool DEFAULT_DATA_ON_SDOUT = true;

/** @} */

}  // namespace ADS9324_CFG
