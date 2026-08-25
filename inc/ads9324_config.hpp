/**
 * @file ads9324_config.hpp
 * @brief Compile-time defaults for the ADS9324 driver
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_config
 */
#pragma once

#include "ads9324_types.hpp"

namespace ADS9324_CFG {

/**
 * @defgroup ads9324_config Compile-Time Configuration
 * @ingroup ads9324_driver
 */

#ifdef CONFIG_ADS9324_RANGE_12V5
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus12V5;
#elif defined(CONFIG_ADS9324_RANGE_10V)
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus10V;
#elif defined(CONFIG_ADS9324_RANGE_2V5)
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus2V5;
#else
inline constexpr ads9324::InputRange DEFAULT_RANGE = ads9324::InputRange::PlusMinus5V;
#endif

#ifdef CONFIG_ADS9324_SINGLE_ENDED
inline constexpr ads9324::InputType DEFAULT_INPUT_TYPE = ads9324::InputType::SingleEnded;
#else
inline constexpr ads9324::InputType DEFAULT_INPUT_TYPE = ads9324::InputType::Differential;
#endif

#ifdef CONFIG_ADS9324_PGA_WIDE
inline constexpr ads9324::PgaBandwidth DEFAULT_BANDWIDTH = ads9324::PgaBandwidth::Wide;
#else
inline constexpr ads9324::PgaBandwidth DEFAULT_BANDWIDTH = ads9324::PgaBandwidth::Low;
#endif

#ifdef CONFIG_ADS9324_OFFSET_BINARY
inline constexpr ads9324::DataFormat DEFAULT_FORMAT = ads9324::DataFormat::OffsetBinary;
#else
inline constexpr ads9324::DataFormat DEFAULT_FORMAT = ads9324::DataFormat::TwosComplement;
#endif

#ifdef CONFIG_ADS9324_SKIP_DEVICE_ID_CHECK
inline constexpr bool REQUIRE_DEVICE_ID = false;
#else
inline constexpr bool REQUIRE_DEVICE_ID = true;
#endif

#ifdef CONFIG_ADS9324_POWER_ON_DELAY_MS
inline constexpr uint32_t POWER_ON_DELAY_MS = CONFIG_ADS9324_POWER_ON_DELAY_MS;
#else
inline constexpr uint32_t POWER_ON_DELAY_MS = 30;  ///< SBASB22 Table 7-18
#endif

#ifdef CONFIG_ADS9324_RESET_DELAY_MS
inline constexpr uint32_t RESET_DELAY_MS = CONFIG_ADS9324_RESET_DELAY_MS;
#else
inline constexpr uint32_t RESET_DELAY_MS = 1;
#endif

#ifdef CONFIG_ADS9324_DRDY_TIMEOUT_US
inline constexpr uint32_t DRDY_TIMEOUT_US = CONFIG_ADS9324_DRDY_TIMEOUT_US;
#else
inline constexpr uint32_t DRDY_TIMEOUT_US = 1000;
#endif

#ifdef CONFIG_ADS9324_CONV_FALLBACK_US
inline constexpr uint32_t CONV_FALLBACK_US = CONFIG_ADS9324_CONV_FALLBACK_US;
#else
inline constexpr uint32_t CONV_FALLBACK_US = 10;  ///< Used when DRDY pin is not wired
#endif

inline constexpr ads9324::DoutLaneMode DEFAULT_LANES = ads9324::DoutLaneMode::Lane1;
inline constexpr ads9324::DoutLength DEFAULT_LENGTH = ads9324::DoutLength::Bits16;
inline constexpr bool DEFAULT_DATA_ON_SDOUT = true;

}  // namespace ADS9324_CFG
