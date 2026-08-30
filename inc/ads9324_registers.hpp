/**
 * @file ads9324_registers.hpp
 * @brief SPI frame helpers and register map for ADS9324 (TI SBASB22)
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_registers
 *
 * @details
 * Configuration SPI is always a **24-bit frame**: 8-bit address + 16-bit data,
 * latched on SCLK rising, decoded on CS rising (SBASB22 §7.5). Address space
 * is banked; write BANK_SEL before accessing channel-bank registers.
 *
 * Channel-bank offsets are identical in AIN1–8 and AIN9–16. Use
 * BankForChannel() and LocalChannel() to map 0-based API channels.
 */
#pragma once

#include <cstdint>
#include "ads9324_types.hpp"

namespace ads9324 {
namespace reg {

/**
 * @defgroup ads9324_registers Register and Frame Constants
 * @ingroup ads9324_driver
 * @brief Addresses, bitfields, and pack/unpack helpers for 24-bit config SPI.
 * @{
 */

/** Bytes per configuration SPI frame. */
inline constexpr uint8_t  kSpiFrameBytes = 3;
/** Mask for the 24-bit payload carried in a `uint32_t`. */
inline constexpr uint32_t kSpiFrameMask  = 0x00FFFFFFu;

// ---- Common bank (BANK_SEL = 0x0001) ---------------------------------------
inline constexpr uint8_t GEN_CFG1         = 0x01;  ///< SW_RST, REG_RD_EN, REG_RD_ADD.
inline constexpr uint8_t BANK_SEL         = 0x02;  ///< Selects common / AIN1–8 / AIN9–16 bank.
inline constexpr uint8_t DIAG_CTRL        = 0x07;  ///< Diagnostic control.
inline constexpr uint8_t PDN_CTL          = 0x08;  ///< Device power-down.
inline constexpr uint8_t GEN_CFG2         = 0x09;  ///< General configuration 2.
inline constexpr uint8_t GEN_CFG3         = 0x0A;  ///< Lanes, word length, SDOUT data, offset-binary.
inline constexpr uint8_t XOR_BITS_CTL     = 0x0B;  ///< Data-line XOR invert.
inline constexpr uint8_t DRDY_ALARM_SEL   = 0x0C;  ///< DRDY/ALARM pin mux + polarity.
inline constexpr uint8_t GEN_CFG4         = 0x0D;  ///< General configuration 4.
inline constexpr uint8_t DIG_DELAY_CFG1   = 0x0E;  ///< Digital delay 1.
inline constexpr uint8_t DIG_DELAY_CFG2   = 0x0F;  ///< Digital delay 2.
inline constexpr uint8_t ANA_CFG1         = 0x10;  ///< Reference select / external REF.
inline constexpr uint8_t ANA_CFG2         = 0x11;  ///< ADC_NUM_SEL / CH_SEL (channel-count mode).
inline constexpr uint8_t ADC_CAL          = 0x12;  ///< ADC calibration control.
inline constexpr uint8_t DIG_FILTER       = 0x14;  ///< Digital filter.
inline constexpr uint8_t GEN_CFG5         = 0x15;  ///< General configuration 5.
inline constexpr uint8_t DEVICE_STATUS    = 0x1D;  ///< Status including CALIB_BUSY.
inline constexpr uint8_t DEVICE_ID        = 0x21;  ///< Reset value @ref kDeviceIdAds9324 (0x0004).

inline constexpr uint16_t BANK_COMMON  = 0x0001;  ///< BANK_SEL value — common bank.
inline constexpr uint16_t BANK_AIN1_8  = 0x0002;  ///< BANK_SEL value — AIN1–AIN8.
inline constexpr uint16_t BANK_AIN9_16 = 0x0004;  ///< BANK_SEL value — AIN9–AIN16.

inline constexpr uint16_t GEN_CFG1_SW_RST    = 0x0002;  ///< Software reset bit.
inline constexpr uint16_t GEN_CFG1_REG_RD_EN = 0x0001;  ///< Enable register-read command.

inline constexpr uint16_t PDN_CTL_DEVICE_PDN = 0x0001;  ///< Whole-device power-down.

/**
 * GEN_CFG3 reset-time bring-up: 1-lane, 16-bit, ADC data on SDOUT
 * (SBASB22 Table 7-18). Matches GenCfg3Value() for 1-lane, 16-bit, SDOUT, two's-complement.
 */
inline constexpr uint16_t GEN_CFG3_1LANE_SDOUT_16B = 0x0032;

inline constexpr uint16_t GEN_CFG3_EN_OFS_BINARY     = (1u << 12);  ///< Offset-binary coding.
inline constexpr uint16_t GEN_CFG3_EN_DIAG_FLAG      = (1u << 8);   ///< Diagnostic flag on data.
inline constexpr uint16_t GEN_CFG3_ADC_DATA_SDOUT_EN = (1u << 1);   ///< Conversion data on SDOUT.

inline constexpr uint16_t ANA_CFG1_REFSEL_CTRL_DIS = (1u << 1);  ///< Disable REFSEL pin override.
inline constexpr uint16_t ANA_CFG1_EXT_REF_EN      = (1u << 0);  ///< External reference enable.

inline constexpr uint16_t DEVICE_STATUS_CALIB_BUSY = (1u << 3);  ///< Calibration in progress.

// ---- Channel banks (same offsets in AIN1–8 and AIN9–16) --------------------
inline constexpr uint8_t PGA_CONFIG_AIN12 = 0x08;  ///< Pair register: even local ch in low byte, odd in high.
inline constexpr uint8_t PGA_BW_SEL       = 0x0C;  ///< 2 bits per local channel.
inline constexpr uint8_t PHASE_DELAY_12   = 0x0D;  ///< Pair phase-delay registers.
inline constexpr uint8_t OFS_AIN1         = 0x11;  ///< OFS_AIN(local) = OFS_AIN1 + local (10-bit two's).
inline constexpr uint8_t GAN_AIN1         = 0x19;  ///< GAN_AIN(local) = GAN_AIN1 + local (14-bit).
inline constexpr uint8_t DWC_CFG          = 0x21;  ///< Window-comparator enable bits + STAT_RST.
inline constexpr uint8_t DWC_TH_AIN1      = 0x22;  ///< High/low 8-bit thresholds packed per channel.
inline constexpr uint8_t DWC_HYS_12       = 0x2A;  ///< Pair hysteresis registers.

inline constexpr uint16_t DWC_CFG_STAT_RST = (1u << 15);  ///< Clear DWC status.

/**
 * @brief Pack a 24-bit configuration write frame.
 * @param addr 8-bit register address in the **currently selected** bank.
 * @param data 16-bit payload.
 * @return Bits [23:16]=addr, [15:0]=data.
 */
[[nodiscard]] inline constexpr uint32_t PackWrite(uint8_t addr, uint16_t data) noexcept {
  return (static_cast<uint32_t>(addr) << 16) | static_cast<uint32_t>(data);
}

/**
 * @brief Extract the 16-bit register payload from a 24-bit MISO frame.
 * @param rx24 24-bit word assembled MSB-first from three SPI bytes.
 * @return Data field (SBASB22 register-read timing: data on the following frame).
 */
[[nodiscard]] inline constexpr uint16_t UnpackRegData(uint32_t rx24) noexcept {
  return static_cast<uint16_t>((rx24 >> 8) & 0xFFFFu);
}

/**
 * @brief Build GEN_CFG1 payload that requests a register read of @p addr.
 * @param addr Target address (REG_RD_ADD in the high byte of the 16-bit field).
 * @return Value to write to GEN_CFG1 (includes @ref GEN_CFG1_REG_RD_EN).
 * @see SBASB22 Table 7-17.
 */
[[nodiscard]] inline constexpr uint16_t ReadCommand(uint8_t addr) noexcept {
  return static_cast<uint16_t>((static_cast<uint16_t>(addr) << 8) | GEN_CFG1_REG_RD_EN);
}

/**
 * @brief BANK_SEL payload for a @ref RegisterBank.
 * @param bank Target bank.
 * @return One of @ref BANK_COMMON, @ref BANK_AIN1_8, @ref BANK_AIN9_16.
 */
[[nodiscard]] inline constexpr uint16_t BankSelectValue(RegisterBank bank) noexcept {
  switch (bank) {
    case RegisterBank::Ain1_8:  return BANK_AIN1_8;
    case RegisterBank::Ain9_16: return BANK_AIN9_16;
    case RegisterBank::Common:
    default:                    return BANK_COMMON;
  }
}

/**
 * @brief Channel-bank for a 0-based API channel.
 * @param ch0 Channel 0–15 (AIN1–AIN16).
 * @return Ain1_8 for 0–7, Ain9_16 for 8–15.
 */
[[nodiscard]] inline constexpr RegisterBank BankForChannel(uint8_t ch0) noexcept {
  return (ch0 < 8) ? RegisterBank::Ain1_8 : RegisterBank::Ain9_16;
}

/**
 * @brief Local 0..7 index inside the selected channel bank.
 * @param ch0 0-based API channel.
 * @return @p ch0 & 7.
 */
[[nodiscard]] inline constexpr uint8_t LocalChannel(uint8_t ch0) noexcept {
  return static_cast<uint8_t>(ch0 & 0x07u);
}

/**
 * @brief PGA_CONFIG pair-register address for a local channel.
 * @param local Bank-local index 0–7.
 * @return @ref PGA_CONFIG_AIN12 + floor(local/2).
 */
[[nodiscard]] inline constexpr uint8_t PgaConfigAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(PGA_CONFIG_AIN12 + (local / 2u));
}

/**
 * @brief OFS_AINx address for a local channel.
 * @param local Bank-local index 0–7.
 */
[[nodiscard]] inline constexpr uint8_t OfsAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(OFS_AIN1 + local);
}

/**
 * @brief GAN_AINx address for a local channel.
 * @param local Bank-local index 0–7.
 */
[[nodiscard]] inline constexpr uint8_t GanAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(GAN_AIN1 + local);
}

/**
 * @brief DWC_TH_AINx address for a local channel.
 * @param local Bank-local index 0–7.
 */
[[nodiscard]] inline constexpr uint8_t DwcThAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(DWC_TH_AIN1 + local);
}

/**
 * @brief Encode one channel's PGA nibble (CME | CM_RANGE | INPUT_RANGE).
 *
 * Even local channels occupy bits [7:0] of the pair register; odd channels
 * occupy bits [15:8]. The caller merges with a read-modify-write.
 *
 * @param cfg Channel programming.
 * @return 8-bit nibble in the low byte (CME in bit 7).
 */
[[nodiscard]] inline constexpr uint16_t EncodePgaNibble(const ChannelConfig& cfg) noexcept {
  const uint16_t cme = cfg.common_mode_correct ? 1u : 0u;
  const uint16_t cm  = static_cast<uint16_t>(cfg.type) & 0x7u;
  const uint16_t ir  = static_cast<uint16_t>(cfg.range) & 0x7u;
  return static_cast<uint16_t>((cme << 7) | (cm << 4) | ir);
}

/**
 * @brief Build GEN_CFG3 from lane/length/SDOUT/coding fields.
 * @param lanes DOUT_LANES (bits [5:4]).
 * @param length DOUT_LENGTH (bits [3:2]).
 * @param data_on_sdout Sets @ref GEN_CFG3_ADC_DATA_SDOUT_EN.
 * @param format Sets @ref GEN_CFG3_EN_OFS_BINARY when OffsetBinary.
 * @return 16-bit GEN_CFG3 value.
 */
[[nodiscard]] inline constexpr uint16_t GenCfg3Value(DoutLaneMode lanes,
                                                     DoutLength length,
                                                     bool data_on_sdout,
                                                     DataFormat format) noexcept {
  uint16_t v = 0;
  if (format == DataFormat::OffsetBinary) {
    v |= GEN_CFG3_EN_OFS_BINARY;
  }
  v |= static_cast<uint16_t>((static_cast<uint16_t>(lanes) & 0x3u) << 4);
  v |= static_cast<uint16_t>((static_cast<uint16_t>(length) & 0x3u) << 2);
  if (data_on_sdout) {
    v |= GEN_CFG3_ADC_DATA_SDOUT_EN;
  }
  return v;
}

/**
 * @brief Build ANA_CFG2 (channel-count mode + CH_SEL).
 * @param num ADC_NUM_SEL.
 * @param ch_sel Group select (low 3 bits).
 * @return 16-bit ANA_CFG2 value.
 */
[[nodiscard]] inline constexpr uint16_t AnaCfg2Value(ChannelCountMode num,
                                                     uint8_t ch_sel) noexcept {
  return static_cast<uint16_t>(((static_cast<uint16_t>(ch_sel) & 0x7u) << 4) |
                               ((static_cast<uint16_t>(num) & 0x3u) << 2));
}

/** @} */

}  // namespace reg
}  // namespace ads9324
