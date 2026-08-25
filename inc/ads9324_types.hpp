/**
 * @file ads9324_types.hpp
 * @brief Public types for the ADS9324 driver
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_types
 *
 * Channel indices in the public API are 0-based (channel 0 = hardware AIN1,
 * channel 15 = hardware AIN16) so the driver matches BaseAdc / other hf ADCs.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace ads9324 {

/**
 * @defgroup ads9324_types Driver Types
 * @ingroup ads9324_driver
 * @brief Public enums, result payloads, and helper utilities.
 */

/** @ingroup ads9324_types */
inline constexpr uint8_t kNumChannels = 16;
/** @ingroup ads9324_types */
inline constexpr uint8_t kResolutionBits = 16;
/** @ingroup ads9324_types */
inline constexpr uint16_t kMaxCountUnsigned = 65535u;
/** @ingroup ads9324_types */
inline constexpr uint16_t kDeviceIdAds9324 = 0x0004u;

/** @ingroup ads9324_types */
enum class Error : uint8_t {
  Ok = 0,
  NotInitialized,
  InvalidChannel,
  InvalidParameter,
  SpiError,
  Timeout,
  DeviceIdMismatch,
  ProgrammingFailed,
};

/** @ingroup ads9324_types */
enum class RegisterBank : uint8_t {
  Common = 0,   ///< BANK_SEL = 0x0001
  Ain1_8 = 1,   ///< BANK_SEL = 0x0002
  Ain9_16 = 2,  ///< BANK_SEL = 0x0004
};

/** @ingroup ads9324_types */
enum class InputType : uint8_t {
  Differential = 0,           ///< CM_RANGE = 0b000
  SingleEnded = 5,            ///< CM_RANGE = 0b101
  SingleEndedOpenWireSafe = 6 ///< CM_RANGE = 0b110
};

/**
 * @ingroup ads9324_types
 * @brief Analog full-scale range encoded in INPUT_RANGE_AINx[2:0] (SBASB22 Table 7-1).
 */
enum class InputRange : uint8_t {
  PlusMinus5V    = 0,  ///< ±5 V
  PlusMinus2V5   = 2,  ///< ±2.5 V
  PlusMinus6V25  = 3,  ///< ±6.25 V
  PlusMinus10V   = 4,  ///< ±10 V
  PlusMinus12V5  = 5,  ///< ±12.5 V
};

/** @ingroup ads9324_types */
enum class PgaBandwidth : uint8_t {
  Low  = 0,  ///< ~25.5 kHz LPF
  Wide = 1,  ///< 280–350 kHz depending on range
};

/** @ingroup ads9324_types */
enum class DataFormat : uint8_t {
  TwosComplement = 0,  ///< Default (EN_OFS_BINARY = 0)
  OffsetBinary   = 1,  ///< EN_OFS_BINARY = 1
};

/** @ingroup ads9324_types */
enum class DoutLaneMode : uint8_t {
  Lanes8 = 0,  ///< D[7:0]
  Lanes4 = 1,  ///< D[7:4]
  Lanes2 = 2,  ///< D[7:6]
  Lane1  = 3,  ///< D7 or SDOUT
};

/** @ingroup ads9324_types */
enum class DoutLength : uint8_t {
  Bits16 = 0,
  Bits24 = 2,
};

/** @ingroup ads9324_types */
enum class ChannelCountMode : uint8_t {
  All16 = 0,  ///< ADC_NUM_SEL = 00b
  Eight = 1,
  Four  = 2,
  Two   = 3,
};

/** @ingroup ads9324_types */
enum class AlarmPinMode : uint8_t {
  Drdy         = 0x0,
  DwcAlert     = 0x1,
  AdcCalDone   = 0x6,
  OrMasked     = 0x8,
};

/** @ingroup ads9324_types */
struct ChannelConfig {
  InputType type           = InputType::Differential;
  InputRange range         = InputRange::PlusMinus5V;
  PgaBandwidth bandwidth   = PgaBandwidth::Low;
  bool common_mode_correct = false;
};

/** @ingroup ads9324_types */
struct ReadResult {
  uint8_t channel = 0;      ///< 0-based channel
  int16_t count   = 0;      ///< Two's-complement 16-bit code
  uint16_t raw    = 0;      ///< Wire word as received
  float voltage   = 0.0f;   ///< Converted using the channel's programmed range
  Error error     = Error::Ok;

  [[nodiscard]] bool ok() const noexcept { return error == Error::Ok; }
};

/** @ingroup ads9324_types */
struct Snapshot {
  int16_t count[kNumChannels]{};
  float voltage[kNumChannels]{};
  uint16_t valid_mask = 0;  ///< Bit N set if channel N was clocked out
  Error error = Error::Ok;

  [[nodiscard]] bool ok() const noexcept { return error == Error::Ok; }
  [[nodiscard]] bool hasChannel(uint8_t ch) const noexcept {
    return ch < kNumChannels && ((valid_mask & (uint16_t{1} << ch)) != 0);
  }
};

/** @ingroup ads9324_types */
struct Diagnostics {
  bool initialized = false;
  uint16_t device_id = 0;
  DataFormat format = DataFormat::TwosComplement;
  DoutLaneMode lanes = DoutLaneMode::Lane1;
  bool data_on_sdout = true;
  uint32_t snapshots = 0;
  uint32_t errors = 0;
};

/**
 * @ingroup ads9324_types
 * @brief Full-scale voltage for an INPUT_RANGE encoding.
 */
[[nodiscard]] inline constexpr float FullScaleVolts(InputRange range) noexcept {
  switch (range) {
    case InputRange::PlusMinus2V5:  return 2.5f;
    case InputRange::PlusMinus5V:    return 5.0f;
    case InputRange::PlusMinus6V25:  return 6.25f;
    case InputRange::PlusMinus10V:   return 10.0f;
    case InputRange::PlusMinus12V5:  return 12.5f;
  }
  return 5.0f;
}

/**
 * @ingroup ads9324_types
 * @brief Convert a two's-complement 16-bit code to volts at the given bipolar FS.
 *
 * +FS − 1.5 LSB maps near +32767; −FS maps near −32768 (SBASB22 Figure 7-4).
 */
[[nodiscard]] inline constexpr float TwosToVoltage(int16_t code, float full_scale) noexcept {
  return (static_cast<float>(code) * full_scale) / 32768.0f;
}

/** @ingroup ads9324_types */
[[nodiscard]] inline constexpr int16_t OffsetBinaryToTwos(uint16_t raw) noexcept {
  return static_cast<int16_t>(static_cast<int32_t>(raw) - 32768);
}

/** @ingroup ads9324_types */
[[nodiscard]] inline constexpr int16_t WireWordToTwos(uint16_t raw, DataFormat format) noexcept {
  if (format == DataFormat::OffsetBinary) {
    return OffsetBinaryToTwos(raw);
  }
  return static_cast<int16_t>(raw);
}

}  // namespace ads9324
