/**
 * @file ads9324_types.hpp
 * @brief Public types for the ADS9324 driver
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_types
 *
 * @details
 * Channel indices in the public API are **0-based** (channel 0 = hardware AIN1,
 * channel 15 = hardware AIN16) so the driver matches `BaseAdc` and other hf
 * ADCs. Hardware register names stay 1-based (AIN1–AIN16) as in TI SBASB22.
 *
 * Conversion data on the wire is 16-bit two's-complement by default
 * (EN_OFS_BINARY = 0). Offset-binary is optional via DataFormat.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace ads9324 {

/**
 * @defgroup ads9324_types Driver Types
 * @ingroup ads9324_driver
 * @brief Public enums, result payloads, and helper utilities.
 * @{
 */

/** Number of simultaneous SAR channels (AIN1–AIN16). */
inline constexpr uint8_t kNumChannels = 16;

/** ADC resolution in bits. */
inline constexpr uint8_t kResolutionBits = 16;

/** Maximum unsigned 16-bit code (offset-binary +FS − 1 LSB). */
inline constexpr uint16_t kMaxCountUnsigned = 65535u;

/**
 * DEVICE_ID reset value for ADS9324 (common bank address 0x21).
 * @see SBASB22 register map.
 */
inline constexpr uint16_t kDeviceIdAds9324 = 0x0004u;

/**
 * @brief Driver-level error codes returned in ReadResult and Snapshot.
 */
enum class Error : uint8_t {
  Ok = 0,              ///< Operation succeeded.
  NotInitialized,      ///< @ref ADS9324::EnsureInitialized has not run (or failed).
  InvalidChannel,      ///< Channel index was not in `[0, 15]`.
  InvalidParameter,    ///< Argument out of the legal register encoding.
  SpiError,            ///< Transport reported a failure, or snapshot lacked the channel.
  Timeout,             ///< DRDY did not assert within @ref ADS9324_CFG::DRDY_TIMEOUT_US.
  DeviceIdMismatch,    ///< Read DEVICE_ID was not @ref kDeviceIdAds9324.
  ProgrammingFailed,   ///< Register write/read-modify-write did not complete.
};

/**
 * @brief Configuration register bank selected by BANK_SEL (common bank 0x02).
 *
 * Writes to BANK_SEL take effect immediately; subsequent address bytes are
 * decoded in the selected bank (SBASB22 §7.5).
 */
enum class RegisterBank : uint8_t {
  Common = 0,   ///< BANK_SEL = 0x0001 — GEN_CFGx, DEVICE_ID, PDN_CTL, …
  Ain1_8 = 1,   ///< BANK_SEL = 0x0002 — PGA/OFS/GAN/DWC for AIN1–AIN8
  Ain9_16 = 2,  ///< BANK_SEL = 0x0004 — PGA/OFS/GAN/DWC for AIN9–AIN16
};

/**
 * @brief PGA input topology encoded in CM_RANGE_AINx (SBASB22 Table 7-1).
 */
enum class InputType : uint8_t {
  Differential = 0,            ///< CM_RANGE = 0b000 — fully differential.
  SingleEnded = 5,             ///< CM_RANGE = 0b101 — single-ended vs AINCOM.
  SingleEndedOpenWireSafe = 6  ///< CM_RANGE = 0b110 — open-wire-safe SE.
};

/**
 * @brief Analog full-scale range encoded in INPUT_RANGE_AINx[2:0]
 *        (SBASB22 Table 7-1).
 *
 * All ranges are bipolar. Voltage conversion uses `V = code * FS / 32768`
 * (two's-complement).
 */
enum class InputRange : uint8_t {
  PlusMinus5V    = 0,  ///< ±5 V (default).
  PlusMinus2V5   = 2,  ///< ±2.5 V
  PlusMinus6V25  = 3,  ///< ±6.25 V
  PlusMinus10V   = 4,  ///< ±10 V
  PlusMinus12V5  = 5,  ///< ±12.5 V
};

/**
 * @brief Per-channel analog LPF bandwidth (PGA_BW_SEL, 2 bits per channel).
 */
enum class PgaBandwidth : uint8_t {
  Low  = 0,  ///< ~25.5 kHz LPF (typical).
  Wide = 1,  ///< 280–350 kHz depending on programmed range.
};

/**
 * @brief Conversion data coding on SDOUT / D[7:0] (GEN_CFG3.EN_OFS_BINARY).
 */
enum class DataFormat : uint8_t {
  TwosComplement = 0,  ///< Default. Midscale = 0x0000.
  OffsetBinary   = 1,  ///< Midscale = 0x8000. Convert with @ref OffsetBinaryToTwos.
};

/**
 * @brief Number of parallel data lanes (GEN_CFG3.DOUT_LANES).
 *
 * This driver clocks conversion data on **SDOUT in 1-lane mode**. Multi-lane
 * D[7:0] capture is a future host DMA path; @ref DoutLaneMode::Lane1 is the
 * supported bring-up setting.
 */
enum class DoutLaneMode : uint8_t {
  Lanes8 = 0,  ///< D[7:0] (not implemented as a capture path).
  Lanes4 = 1,  ///< D[7:4]
  Lanes2 = 2,  ///< D[7:6]
  Lane1  = 3,  ///< D7 or SDOUT — **supported path**.
};

/**
 * @brief Conversion word length on the data interface (GEN_CFG3.DOUT_LENGTH).
 */
enum class DoutLength : uint8_t {
  Bits16 = 0,  ///< 16-bit words (default; 32 bytes for 16 channels).
  Bits24 = 2,  ///< 24-bit words; driver still uses the high 16 bits as the code.
};

/**
 * @brief How many channels are clocked out after CONVST (ANA_CFG2.ADC_NUM_SEL).
 *
 * Snapshot `valid_mask` bit N is set only for channels that were actually
 * clocked. With @ref ChannelCountMode::All16, bits 0–15 are all set on success.
 */
enum class ChannelCountMode : uint8_t {
  All16 = 0,  ///< ADC_NUM_SEL = 00b — all 16 channels (default).
  Eight = 1,  ///< First 8 channels of the selected group.
  Four  = 2,  ///< First 4 channels of the selected group.
  Two   = 3,  ///< First 2 channels of the selected group.
};

/**
 * @brief Function muxed onto the DRDY/ALARM pin (DRDY_ALARM_SEL[3:0]).
 */
enum class AlarmPinMode : uint8_t {
  Drdy         = 0x0,  ///< Conversion-complete (default).
  DwcAlert     = 0x1,  ///< Digital window comparator aggregate alert.
  AdcCalDone   = 0x6,  ///< ADC calibration complete.
  OrMasked     = 0x8,  ///< OR of masked status bits.
};

/**
 * @brief Per-channel PGA programming (written as a nibble in PGA_CONFIG_AINxy).
 */
struct ChannelConfig {
  InputType type           = InputType::Differential;  ///< Differential / SE / open-wire-safe.
  InputRange range         = InputRange::PlusMinus5V;  ///< Bipolar full-scale.
  PgaBandwidth bandwidth   = PgaBandwidth::Low;        ///< Analog LPF.
  bool common_mode_correct = false;                    ///< CME bit in the PGA nibble.
};

/**
 * @brief Result of @ref ADS9324::ReadChannel (one channel after a full snapshot).
 */
struct ReadResult {
  uint8_t channel = 0;      ///< 0-based API channel.
  int16_t count   = 0;      ///< Two's-complement 16-bit code.
  uint16_t raw    = 0;      ///< Wire word as received (before format conversion in some paths).
  float voltage   = 0.0f;   ///< Converted using the channel's programmed @ref InputRange.
  Error error     = Error::Ok;  ///< @ref Error::Ok when the sample is valid.

  /** @return true when @ref error equals @ref Error::Ok. */
  [[nodiscard]] bool ok() const noexcept { return error == Error::Ok; }
};

/**
 * @brief Simultaneous conversion of all enabled channels.
 *
 * Arrays are indexed by 0-based API channel. Entries outside @ref valid_mask
 * are left at zero and must not be treated as samples.
 */
struct Snapshot {
  int16_t count[kNumChannels]{};   ///< Two's-complement codes.
  float voltage[kNumChannels]{};   ///< Engineering units (volts).
  uint16_t valid_mask = 0;         ///< Bit N set if channel N was clocked out.
  Error error = Error::Ok;         ///< @ref Error::Ok on a complete capture.

  /** @return true when @ref error equals @ref Error::Ok. */
  [[nodiscard]] bool ok() const noexcept { return error == Error::Ok; }

  /**
   * @brief Test whether channel @p ch was present in this snapshot.
   * @param ch 0-based API channel.
   * @return true if @p ch is in range and bit @p ch of @ref valid_mask is set.
   */
  [[nodiscard]] bool hasChannel(uint8_t ch) const noexcept {
    return ch < kNumChannels && ((valid_mask & (uint16_t{1} << ch)) != 0);
  }
};

/**
 * @brief Lightweight driver health counters (no SPI traffic).
 */
struct Diagnostics {
  bool initialized = false;                          ///< @ref ADS9324::IsInitialized.
  uint16_t device_id = 0;                            ///< Last DEVICE_ID if cached by the host.
  DataFormat format = DataFormat::TwosComplement;    ///< Programmed coding.
  DoutLaneMode lanes = DoutLaneMode::Lane1;          ///< Programmed lane mode.
  bool data_on_sdout = true;                         ///< GEN_CFG3 ADC_DATA_SDOUT_EN.
  uint32_t snapshots = 0;                            ///< Successful @ref ADS9324::ReadSnapshot calls.
  uint32_t errors = 0;                               ///< Failed init / timeout / programming counts.
};

/**
 * @brief Full-scale voltage magnitude for an INPUT_RANGE encoding.
 * @param range Programmed bipolar range.
 * @return Positive FS in volts (e.g. 10.0f for ±10 V). Unknown encodings return 5.0f.
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
 * @brief Convert a two's-complement 16-bit code to volts at the given bipolar FS.
 *
 * +FS − 1.5 LSB maps near +32767; −FS maps near −32768 (SBASB22 Figure 7-4).
 *
 * @param code Signed ADC code.
 * @param full_scale Positive full-scale voltage (from @ref FullScaleVolts).
 * @return Voltage in volts.
 */
[[nodiscard]] inline constexpr float TwosToVoltage(int16_t code, float full_scale) noexcept {
  return (static_cast<float>(code) * full_scale) / 32768.0f;
}

/**
 * @brief Convert an offset-binary wire word to two's-complement.
 * @param raw 16-bit offset-binary sample (midscale 0x8000).
 * @return Signed code.
 */
[[nodiscard]] inline constexpr int16_t OffsetBinaryToTwos(uint16_t raw) noexcept {
  return static_cast<int16_t>(static_cast<int32_t>(raw) - 32768);
}

/**
 * @brief Interpret a 16-bit wire word according to the programmed data format.
 * @param raw Wire word (MSB first).
 * @param format @ref DataFormat::TwosComplement or @ref DataFormat::OffsetBinary.
 * @return Signed two's-complement code.
 */
[[nodiscard]] inline constexpr int16_t WireWordToTwos(uint16_t raw, DataFormat format) noexcept {
  if (format == DataFormat::OffsetBinary) {
    return OffsetBinaryToTwos(raw);
  }
  return static_cast<int16_t>(raw);
}

/** @} */

}  // namespace ads9324
