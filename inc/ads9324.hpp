/**
 * @file ads9324.hpp
 * @brief Main driver class for the TI ADS9324 16-ch 16-bit simultaneous SAR ADC
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_driver
 *
 * @details
 * Hardware-agnostic C++20 driver for the Texas Instruments ADS9324 (SBASB22).
 * Each channel has an integrated PGA (differential / single-ended / open-wire
 * safe), analog LPF, and on-chip offset + gain calibration. Conversion is
 * simultaneous; CONVST falling edge samples all enabled channels. The default
 * digital path is **1-lane 16-bit data on SDOUT** so a single SPI controller
 * (ESP32-C6, STM32 SPI5, …) can both program registers and read conversions.
 *
 * Multi-lane D[7:0] capture is a future host DMA path — GEN_CFG3 can still be
 * programmed, but this driver clocks conversion data on SDOUT.
 *
 * @section ads9324_hpp_usage Basic Usage
 * @code
 * class MySpi : public ads9324::SpiInterface<MySpi> {
 * public:
 *   void transfer(const uint8_t* tx, uint8_t* rx, size_t len) { ... }
 * };
 * class MyHost : public ads9324::HostInterface<MyHost> {
 * public:
 *   void ConvstWrite(bool high) { ... }
 *   bool DrdyRead() { return true; }
 *   bool HasDrdy() const { return false; }
 *   void DelayUs(uint32_t us) { ... }
 *   void DelayMs(uint32_t ms) { ... }
 * };
 *
 * MySpi spi; MyHost host;
 * ads9324::ADS9324<MySpi, MyHost> adc(spi, host);
 * adc.EnsureInitialized();
 * adc.ConfigureChannel(0, {ads9324::InputType::Differential,
 *                          ads9324::InputRange::PlusMinus10V});
 * auto snap = adc.ReadSnapshot();
 * @endcode
 */
#pragma once

#define ADS9324_HEADER_INCLUDED

#include <cstdint>
#include <cstddef>
#include <algorithm>

#include "ads9324_config.hpp"
#include "ads9324_registers.hpp"
#include "ads9324_spi_interface.hpp"
#include "ads9324_types.hpp"

namespace ads9324 {

/**
 * @defgroup ads9324_driver ADS9324 Driver
 * @brief Hardware-agnostic C++20 driver for the TI ADS9324 ADC + integrated AFE.
 */

/**
 * @defgroup ads9324_core Core Driver API
 * @ingroup ads9324_driver
 */

/**
 * @ingroup ads9324_core
 * @brief Main driver class for the ADS9324.
 *
 * @tparam SpiType  CRTP SPI (3-byte config frames + conversion burst)
 * @tparam HostType CRTP CONVST / DRDY / delay
 */
template <typename SpiType, typename HostType>
class ADS9324 {
public:
  explicit ADS9324(SpiType& spi, HostType& host) noexcept;

  ADS9324(const ADS9324&) = delete;
  ADS9324& operator=(const ADS9324&) = delete;

  /**
   * @brief Power-on delay, optional SW reset, 1-lane SDOUT mode, DEVICE_ID check.
   * @param force Re-run even if already initialized.
   */
  bool EnsureInitialized(bool force = false) noexcept;

  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

  /** @brief Software reset (GEN_CFG1.SW_RST) and re-apply digital interface. */
  bool SoftwareReset() noexcept;

  /** @brief DEVICE_ID (common bank 0x21). ADS9324 reset value is 0x0004. */
  uint16_t ReadDeviceId() noexcept;

  bool ConfigureChannel(uint8_t channel, const ChannelConfig& cfg) noexcept;
  bool ConfigureAllChannels(const ChannelConfig& cfg) noexcept;
  [[nodiscard]] ChannelConfig GetChannelConfig(uint8_t channel) const noexcept;

  /**
   * @brief Program 10-bit two's-complement offset (OFS_AINx).
   * Hardware applies (code×4 + OFS) / 4 (SBASB22 Equation 1).
   */
  bool SetChannelOffset(uint8_t channel, int16_t ofs_10bit) noexcept;

  /**
   * @brief Program 14-bit gain correction (GAN_AINx). Correction = GAN / 65536.
   */
  bool SetChannelGain(uint8_t channel, uint16_t gan_14bit) noexcept;

  bool SetDataFormat(DataFormat format) noexcept;
  [[nodiscard]] DataFormat GetDataFormat() const noexcept { return format_; }

  bool SetDigitalInterface(DoutLaneMode lanes, DoutLength length, bool data_on_sdout) noexcept;

  bool SetChannelCountMode(ChannelCountMode mode, uint8_t ch_sel = 0) noexcept;

  bool SetPowerDown(bool power_down) noexcept;

  bool SetAlarmPin(AlarmPinMode mode, bool active_low = false) noexcept;

  /**
   * @brief Enable/disable digital window comparator for one channel (DWC_EN_AINx).
   * Thresholds are 8-bit two's-complement high/low in DWC_TH_AINx.
   */
  bool ProgramWindowComparator(uint8_t channel, int8_t high_th, int8_t low_th,
                               bool enable = true) noexcept;

  /**
   * @brief Pulse CONVST, wait DRDY (or fallback delay), clock all enabled channels.
   */
  Snapshot ReadSnapshot() noexcept;

  /**
   * @brief Simultaneous snapshot, then return one channel (0–15).
   */
  ReadResult ReadChannel(uint8_t channel) noexcept;

  [[nodiscard]] float CountToVoltage(uint8_t channel, int16_t twos) const noexcept;

  [[nodiscard]] Diagnostics GetDiagnostics() const noexcept;

  static constexpr uint8_t GetDriverVersionMajor() noexcept { return 1; }
  static constexpr uint8_t GetDriverVersionMinor() noexcept { return 0; }
  static constexpr uint8_t GetDriverVersionPatch() noexcept { return 0; }

private:
  uint32_t spiTransfer24(uint32_t tx24) noexcept;
  bool writeRegister(RegisterBank bank, uint8_t addr, uint16_t data) noexcept;
  bool readRegister(RegisterBank bank, uint8_t addr, uint16_t& data) noexcept;
  bool selectBank(RegisterBank bank) noexcept;
  bool applyDigitalInterface() noexcept;
  bool pulseConvstAndWait() noexcept;
  uint8_t activeChannelCount() const noexcept;

  SpiType& spi_;
  HostType& host_;

  bool initialized_ = false;
  RegisterBank current_bank_ = RegisterBank::Common;
  DataFormat format_ = ADS9324_CFG::DEFAULT_FORMAT;
  DoutLaneMode lanes_ = ADS9324_CFG::DEFAULT_LANES;
  DoutLength length_ = ADS9324_CFG::DEFAULT_LENGTH;
  bool data_on_sdout_ = ADS9324_CFG::DEFAULT_DATA_ON_SDOUT;
  ChannelCountMode ch_count_mode_ = ChannelCountMode::All16;
  uint8_t ch_sel_ = 0;
  ChannelConfig ch_cfg_[kNumChannels]{};
  uint32_t snapshots_ = 0;
  uint32_t errors_ = 0;
};

}  // namespace ads9324

#include "ads9324.ipp"
