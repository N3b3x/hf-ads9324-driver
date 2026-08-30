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
 * can both program registers and read conversions.
 *
 * Multi-lane D[7:0] capture is a future host DMA path — GEN_CFG3 can still be
 * programmed, but this driver clocks conversion data on SDOUT.
 *
 * @section ads9324_hpp_usage Basic Usage
 * @code{.cpp}
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
 *
 * @see ads9324_spi_interface.hpp, ads9324_registers.hpp, SBASB22
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
 * Not thread-safe. The host serializes access if multiple tasks share one instance.
 *
 * @tparam SpiType  CRTP type implementing @ref SpiInterface (3-byte config
 *                  frames + conversion burst).
 * @tparam HostType CRTP type implementing @ref HostInterface (CONVST / DRDY /
 *                  delay).
 */
template <typename SpiType, typename HostType>
class ADS9324 {
public:
  /**
   * @brief Bind SPI and host transports. Does not touch the bus.
   * @param spi  Platform SPI (must outlive this object).
   * @param host Platform CONVST/DRDY/delay (must outlive this object).
   *
   * Channel configs are initialized from @ref ADS9324_CFG defaults. Call
   * @ref EnsureInitialized before conversion.
   */
  explicit ADS9324(SpiType& spi, HostType& host) noexcept;

  ADS9324(const ADS9324&) = delete;
  ADS9324& operator=(const ADS9324&) = delete;

  /**
   * @brief Power-on delay, optional SW reset, 1-lane SDOUT mode, DEVICE_ID check.
   *
   * Sequence: wait @ref ADS9324_CFG::POWER_ON_DELAY_MS, @ref SoftwareReset,
   * BANK_SEL=common, @ref applyDigitalInterface, optional DEVICE_ID compare
   * to @ref kDeviceIdAds9324.
   *
   * @param force Re-run even if already initialized.
   * @retval true  Device is ready for PGA programming and snapshots.
   * @retval false Reset, SPI, or DEVICE_ID check failed.
   */
  bool EnsureInitialized(bool force = false) noexcept;

  /**
   * @brief Whether @ref EnsureInitialized has succeeded.
   * @return true after a successful init (until a failed re-init).
   */
  [[nodiscard]] bool IsInitialized() const noexcept { return initialized_; }

  /**
   * @brief Software reset (GEN_CFG1.SW_RST) and re-apply digital interface.
   * @retval true  Reset write and GEN_CFG3 reprogramming succeeded.
   * @retval false Register write failed.
   */
  bool SoftwareReset() noexcept;

  /**
   * @brief Read DEVICE_ID (common bank 0x21).
   * @return 16-bit ID, or 0 if the read failed. ADS9324 reset value is 0x0004.
   */
  uint16_t ReadDeviceId() noexcept;

  /**
   * @brief Program one channel's PGA type, range, bandwidth, and CME bit.
   *
   * Read-modify-writes the shared PGA_CONFIG pair register (even/odd nibble)
   * and PGA_BW_SEL. API channel 0 = AIN1.
   *
   * @param channel 0–15.
   * @param cfg     Desired analog front-end.
   * @retval true  Both registers updated and cached in @ref GetChannelConfig.
   * @retval false Channel out of range or SPI failed.
   */
  bool ConfigureChannel(uint8_t channel, const ChannelConfig& cfg) noexcept;

  /**
   * @brief Apply the same @ref ChannelConfig to all 16 channels.
   * @param cfg Shared programming.
   * @retval true  All 16 @ref ConfigureChannel calls succeeded.
   * @retval false Stopped at the first failure.
   */
  bool ConfigureAllChannels(const ChannelConfig& cfg) noexcept;

  /**
   * @brief Last successfully programmed config for a channel (software cache).
   * @param channel 0–15.
   * @return Cached config, or default-constructed @ref ChannelConfig if
   *         @p channel is out of range.
   */
  [[nodiscard]] ChannelConfig GetChannelConfig(uint8_t channel) const noexcept;

  /**
   * @brief Program 10-bit two's-complement offset (OFS_AINx).
   *
   * Hardware applies `(code×4 + OFS) / 4` (SBASB22 Equation 1).
   *
   * @param channel   0–15.
   * @param ofs_10bit Signed offset; only the low 10 bits are written.
   * @retval true  Write accepted.
   * @retval false Invalid channel or SPI failed.
   */
  bool SetChannelOffset(uint8_t channel, int16_t ofs_10bit) noexcept;

  /**
   * @brief Program 14-bit gain correction (GAN_AINx). Correction = GAN / 65536.
   * @param channel   0–15.
   * @param gan_14bit Unsigned gain; only the low 14 bits are written.
   * @retval true  Write accepted.
   * @retval false Invalid channel or SPI failed.
   */
  bool SetChannelGain(uint8_t channel, uint16_t gan_14bit) noexcept;

  /**
   * @brief Set conversion coding and rewrite GEN_CFG3.
   * @param format Two's-complement (default) or offset-binary.
   * @retval true  GEN_CFG3 updated.
   */
  bool SetDataFormat(DataFormat format) noexcept;

  /**
   * @brief Last programmed @ref DataFormat (software cache).
   */
  [[nodiscard]] DataFormat GetDataFormat() const noexcept { return format_; }

  /**
   * @brief Program GEN_CFG3 lane count, word length, and SDOUT data enable.
   *
   * The supported capture path is 1-lane, 16-bit, data on SDOUT. Other lane
   * settings may be written for experiments but @ref ReadSnapshot still clocks
   * SDOUT only.
   *
   * @param lanes         DOUT_LANES field.
   * @param length        16- or 24-bit conversion words.
   * @param data_on_sdout ADC_DATA_SDOUT_EN.
   * @retval true  GEN_CFG3 updated.
   */
  bool SetDigitalInterface(DoutLaneMode lanes, DoutLength length, bool data_on_sdout) noexcept;

  /**
   * @brief How many channels are clocked after CONVST (ANA_CFG2).
   * @param mode   All16 / Eight / Four / Two.
   * @param ch_sel Group select (low 3 bits), used when not All16.
   * @retval true  ANA_CFG2 updated.
   */
  bool SetChannelCountMode(ChannelCountMode mode, uint8_t ch_sel = 0) noexcept;

  /**
   * @brief Device power-down (PDN_CTL.DEVICE_PDN).
   * @param power_down true = power down, false = run.
   * @retval true  Write accepted.
   */
  bool SetPowerDown(bool power_down) noexcept;

  /**
   * @brief Mux the DRDY/ALARM pin (DRDY_ALARM_SEL).
   * @param mode       Pin function.
   * @param active_low true = invert pin polarity (bit 4).
   * @retval true  Write accepted.
   */
  bool SetAlarmPin(AlarmPinMode mode, bool active_low = false) noexcept;

  /**
   * @brief Enable/disable digital window comparator for one channel (DWC_EN_AINx).
   *
   * Thresholds are 8-bit two's-complement high/low packed in DWC_TH_AINx.
   *
   * @param channel 0–15.
   * @param high_th Upper window (signed 8-bit).
   * @param low_th  Lower window (signed 8-bit).
   * @param enable  true = set the channel enable bit in DWC_CFG.
   * @retval true  Threshold and enable bit updated.
   * @retval false Invalid channel or SPI failed.
   */
  bool ProgramWindowComparator(uint8_t channel, int8_t high_th, int8_t low_th,
                               bool enable = true) noexcept;

  /**
   * @brief Pulse CONVST, wait DRDY (or fallback delay), clock all enabled channels.
   * @return Snapshot with @ref Snapshot::valid_mask bits set for clocked
   *         channels. @ref Error::NotInitialized or @ref Error::Timeout on
   *         failure.
   * @pre @ref EnsureInitialized succeeded.
   */
  Snapshot ReadSnapshot() noexcept;

  /**
   * @brief Simultaneous snapshot, then return one channel (0–15).
   *
   * Still converts **all** enabled channels; extra samples are discarded.
   * Prefer @ref ReadSnapshot when more than one channel is needed.
   *
   * @param channel 0–15.
   * @return Per-channel result; @ref Error::InvalidChannel if out of range.
   */
  ReadResult ReadChannel(uint8_t channel) noexcept;

  /**
   * @brief Convert a two's-complement code using the channel's cached range.
   * @param channel 0–15 (out of range uses ±5 V).
   * @param twos    Signed ADC code.
   * @return Voltage in volts.
   */
  [[nodiscard]] float CountToVoltage(uint8_t channel, int16_t twos) const noexcept;

  /**
   * @brief Copy software diagnostics (no SPI).
   * @note @ref Diagnostics::device_id is not filled here; use @ref ReadDeviceId.
   */
  [[nodiscard]] Diagnostics GetDiagnostics() const noexcept;

  /** @brief Driver semantic major version. */
  static constexpr uint8_t GetDriverVersionMajor() noexcept { return 1; }
  /** @brief Driver semantic minor version. */
  static constexpr uint8_t GetDriverVersionMinor() noexcept { return 0; }
  /** @brief Driver semantic patch version. */
  static constexpr uint8_t GetDriverVersionPatch() noexcept { return 0; }

private:
  /**
   * @brief One 24-bit full-duplex config transfer.
   * @param tx24 Packed frame (@ref reg::PackWrite).
   * @return Assembled MISO word (MSB-first).
   */
  uint32_t spiTransfer24(uint32_t tx24) noexcept;

  /**
   * @brief Select @p bank if needed, then write @p addr/@p data.
   * @param bank Target bank.
   * @param addr Register address.
   * @param data 16-bit payload.
   * @retval true  Always today (SPI errors are not yet surfaced by the CRTP).
   */
  bool writeRegister(RegisterBank bank, uint8_t addr, uint16_t data) noexcept;

  /**
   * @brief Two-frame register read (GEN_CFG1 REG_RD then dummy clock).
   * @param bank  Bank containing @p addr.
   * @param addr  Register address in that bank.
   * @param[out] data Unpacked 16-bit payload.
   * @return true after the dummy frame (SPI errors are not yet surfaced).
   * @see SBASB22 Table 7-17.
   */
  bool readRegister(RegisterBank bank, uint8_t addr, uint16_t& data) noexcept;

  /** @brief Write BANK_SEL and cache @ref current_bank_. */
  bool selectBank(RegisterBank bank) noexcept;

  /** @brief Write GEN_CFG3 from cached lane/length/SDOUT/format. */
  bool applyDigitalInterface() noexcept;

  /**
   * @brief CONVST high ≥ 1 µs, falling edge, then DRDY poll or fallback delay.
   * @retval false DRDY timed out.
   */
  bool pulseConvstAndWait() noexcept;

  /** @brief Channel count implied by @ref ch_count_mode_ (2/4/8/16). */
  uint8_t activeChannelCount() const noexcept;

  SpiType& spi_;    ///< CRTP SPI backend.
  HostType& host_;  ///< CRTP CONVST/DRDY/delay backend.

  bool initialized_ = false;  ///< @ref EnsureInitialized succeeded.
  RegisterBank current_bank_ = RegisterBank::Common;  ///< Last BANK_SEL.
  DataFormat format_ = ADS9324_CFG::DEFAULT_FORMAT;
  DoutLaneMode lanes_ = ADS9324_CFG::DEFAULT_LANES;
  DoutLength length_ = ADS9324_CFG::DEFAULT_LENGTH;
  bool data_on_sdout_ = ADS9324_CFG::DEFAULT_DATA_ON_SDOUT;
  ChannelCountMode ch_count_mode_ = ChannelCountMode::All16;
  uint8_t ch_sel_ = 0;  ///< ANA_CFG2 CH_SEL.
  ChannelConfig ch_cfg_[kNumChannels]{};  ///< Last PGA programming per channel.
  uint32_t snapshots_ = 0;  ///< Successful snapshot count.
  uint32_t errors_ = 0;     ///< Failed init / timeout / programming count.
};

}  // namespace ads9324

#include "ads9324.ipp"
