/**
 * @file ads9324_registers.hpp
 * @brief SPI frame helpers and register map for ADS9324 (TI SBASB22)
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_registers
 *
 * Configuration SPI is always a 24-bit frame: 8-bit address + 16-bit data,
 * latched on SCLK rising, decoded on CS rising (SBASB22 §7.5).
 */
#pragma once

#include <cstdint>
#include "ads9324_types.hpp"

namespace ads9324 {
namespace reg {

/**
 * @defgroup ads9324_registers Register and Frame Constants
 * @ingroup ads9324_driver
 */

inline constexpr uint8_t  kSpiFrameBytes = 3;
inline constexpr uint32_t kSpiFrameMask  = 0x00FFFFFFu;

// ---- Common bank (BANK_SEL = 0x0001) ---------------------------------------
inline constexpr uint8_t GEN_CFG1         = 0x01;
inline constexpr uint8_t BANK_SEL         = 0x02;
inline constexpr uint8_t DIAG_CTRL        = 0x07;
inline constexpr uint8_t PDN_CTL          = 0x08;
inline constexpr uint8_t GEN_CFG2         = 0x09;
inline constexpr uint8_t GEN_CFG3         = 0x0A;
inline constexpr uint8_t XOR_BITS_CTL     = 0x0B;
inline constexpr uint8_t DRDY_ALARM_SEL   = 0x0C;
inline constexpr uint8_t GEN_CFG4         = 0x0D;
inline constexpr uint8_t DIG_DELAY_CFG1   = 0x0E;
inline constexpr uint8_t DIG_DELAY_CFG2   = 0x0F;
inline constexpr uint8_t ANA_CFG1         = 0x10;
inline constexpr uint8_t ANA_CFG2         = 0x11;
inline constexpr uint8_t ADC_CAL          = 0x12;
inline constexpr uint8_t DIG_FILTER       = 0x14;
inline constexpr uint8_t GEN_CFG5         = 0x15;
inline constexpr uint8_t DEVICE_STATUS    = 0x1D;
inline constexpr uint8_t DEVICE_ID        = 0x21;

inline constexpr uint16_t BANK_COMMON  = 0x0001;
inline constexpr uint16_t BANK_AIN1_8  = 0x0002;
inline constexpr uint16_t BANK_AIN9_16 = 0x0004;

inline constexpr uint16_t GEN_CFG1_SW_RST    = 0x0002;
inline constexpr uint16_t GEN_CFG1_REG_RD_EN = 0x0001;

inline constexpr uint16_t PDN_CTL_DEVICE_PDN = 0x0001;

/** GEN_CFG3 = 1-lane, 16-bit, ADC data on SDOUT (Table 7-18). */
inline constexpr uint16_t GEN_CFG3_1LANE_SDOUT_16B = 0x0032;

inline constexpr uint16_t GEN_CFG3_EN_OFS_BINARY     = (1u << 12);
inline constexpr uint16_t GEN_CFG3_EN_DIAG_FLAG      = (1u << 8);
inline constexpr uint16_t GEN_CFG3_ADC_DATA_SDOUT_EN = (1u << 1);

inline constexpr uint16_t ANA_CFG1_REFSEL_CTRL_DIS = (1u << 1);
inline constexpr uint16_t ANA_CFG1_EXT_REF_EN      = (1u << 0);

inline constexpr uint16_t DEVICE_STATUS_CALIB_BUSY = (1u << 3);

// ---- Channel banks (same offsets in AIN1–8 and AIN9–16) --------------------
inline constexpr uint8_t PGA_CONFIG_AIN12 = 0x08;  ///< +0..+3 pair registers
inline constexpr uint8_t PGA_BW_SEL       = 0x0C;
inline constexpr uint8_t PHASE_DELAY_12   = 0x0D;
inline constexpr uint8_t OFS_AIN1         = 0x11;  ///< +0..+7
inline constexpr uint8_t GAN_AIN1         = 0x19;  ///< +0..+7
inline constexpr uint8_t DWC_CFG          = 0x21;
inline constexpr uint8_t DWC_TH_AIN1      = 0x22;  ///< +0..+7
inline constexpr uint8_t DWC_HYS_12       = 0x2A;  ///< +0..+3 pair registers

inline constexpr uint16_t DWC_CFG_STAT_RST = (1u << 15);

[[nodiscard]] inline constexpr uint32_t PackWrite(uint8_t addr, uint16_t data) noexcept {
  return (static_cast<uint32_t>(addr) << 16) | static_cast<uint32_t>(data);
}

[[nodiscard]] inline constexpr uint16_t UnpackRegData(uint32_t rx24) noexcept {
  return static_cast<uint16_t>((rx24 >> 8) & 0xFFFFu);
}

[[nodiscard]] inline constexpr uint16_t ReadCommand(uint8_t addr) noexcept {
  return static_cast<uint16_t>((static_cast<uint16_t>(addr) << 8) | GEN_CFG1_REG_RD_EN);
}

[[nodiscard]] inline constexpr uint16_t BankSelectValue(RegisterBank bank) noexcept {
  switch (bank) {
    case RegisterBank::Ain1_8:  return BANK_AIN1_8;
    case RegisterBank::Ain9_16: return BANK_AIN9_16;
    case RegisterBank::Common:
    default:                    return BANK_COMMON;
  }
}

[[nodiscard]] inline constexpr RegisterBank BankForChannel(uint8_t ch0) noexcept {
  return (ch0 < 8) ? RegisterBank::Ain1_8 : RegisterBank::Ain9_16;
}

/** Local 0..7 index inside the selected channel bank. */
[[nodiscard]] inline constexpr uint8_t LocalChannel(uint8_t ch0) noexcept {
  return static_cast<uint8_t>(ch0 & 0x07u);
}

[[nodiscard]] inline constexpr uint8_t PgaConfigAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(PGA_CONFIG_AIN12 + (local / 2u));
}

[[nodiscard]] inline constexpr uint8_t OfsAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(OFS_AIN1 + local);
}

[[nodiscard]] inline constexpr uint8_t GanAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(GAN_AIN1 + local);
}

[[nodiscard]] inline constexpr uint8_t DwcThAddr(uint8_t local) noexcept {
  return static_cast<uint8_t>(DWC_TH_AIN1 + local);
}

[[nodiscard]] inline constexpr uint16_t EncodePgaNibble(const ChannelConfig& cfg) noexcept {
  const uint16_t cme = cfg.common_mode_correct ? 1u : 0u;
  const uint16_t cm  = static_cast<uint16_t>(cfg.type) & 0x7u;
  const uint16_t ir  = static_cast<uint16_t>(cfg.range) & 0x7u;
  return static_cast<uint16_t>((cme << 7) | (cm << 4) | ir);
}

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

[[nodiscard]] inline constexpr uint16_t AnaCfg2Value(ChannelCountMode num,
                                                     uint8_t ch_sel) noexcept {
  return static_cast<uint16_t>(((static_cast<uint16_t>(ch_sel) & 0x7u) << 4) |
                               ((static_cast<uint16_t>(num) & 0x3u) << 2));
}

}  // namespace reg
}  // namespace ads9324
