/**
 * @file ads9324.ipp
 * @brief Template implementation for ads9324::ADS9324
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_core
 *
 * @details
 * Header-only `.ipp` (same pattern as hf-ads7952-driver). Public contracts live
 * in `ads9324.hpp`; this file documents the silicon sequences that are not
 * obvious from the signatures: banked PGA nibble merge, two-frame register
 * read, CONVST/DRDY handshake, and SDOUT burst unpack.
 *
 * @note SPI write helpers currently treat the CRTP `transfer` as infallible
 *       (always return true). Surface transport errors when the platform
 *       adapter reports them.
 */
#pragma once

namespace ads9324 {

template <typename SpiType, typename HostType>
ADS9324<SpiType, HostType>::ADS9324(SpiType& spi, HostType& host) noexcept
    : spi_(spi), host_(host) {
  const ChannelConfig def{
      ADS9324_CFG::DEFAULT_INPUT_TYPE,
      ADS9324_CFG::DEFAULT_RANGE,
      ADS9324_CFG::DEFAULT_BANDWIDTH,
      false};
  for (uint8_t i = 0; i < kNumChannels; ++i) {
    ch_cfg_[i] = def;
  }
}

/** Pack @p tx24 into three bytes, full-duplex, reassemble MISO MSB-first. */
template <typename SpiType, typename HostType>
uint32_t ADS9324<SpiType, HostType>::spiTransfer24(uint32_t tx24) noexcept {
  uint8_t tx[3] = {
      static_cast<uint8_t>((tx24 >> 16) & 0xFFu),
      static_cast<uint8_t>((tx24 >> 8) & 0xFFu),
      static_cast<uint8_t>(tx24 & 0xFFu),
  };
  uint8_t rx[3] = {0, 0, 0};
  spi_.transfer(tx, rx, 3);
  return (static_cast<uint32_t>(rx[0]) << 16) | (static_cast<uint32_t>(rx[1]) << 8) |
         static_cast<uint32_t>(rx[2]);
}

/** BANK_SEL is in the common bank; writing it switches decode for later addresses. */
template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::selectBank(RegisterBank bank) noexcept {
  const uint32_t rx =
      spiTransfer24(reg::PackWrite(reg::BANK_SEL, reg::BankSelectValue(bank)));
  (void)rx;
  current_bank_ = bank;
  return true;
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::writeRegister(RegisterBank bank, uint8_t addr,
                                               uint16_t data) noexcept {
  if (addr != reg::BANK_SEL && current_bank_ != bank) {
    if (!selectBank(bank)) {
      return false;
    }
  }
  spiTransfer24(reg::PackWrite(addr, data));
  if (addr == reg::BANK_SEL) {
    current_bank_ = bank;
  }
  return true;
}

// Two-frame read: write GEN_CFG1 with REG_RD_ADD|REG_RD_EN, then clock a dummy
// 24-bit frame. Payload is bits [23:8] of the second MISO word (Table 7-17).
template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::readRegister(RegisterBank bank, uint8_t addr,
                                              uint16_t& data) noexcept {
  if (!selectBank(bank)) {
    return false;
  }
  // Frame: write GEN_CFG1 with REG_RD_ADD and REG_RD_EN (Table 7-17).
  spiTransfer24(reg::PackWrite(reg::GEN_CFG1, reg::ReadCommand(addr)));
  const uint32_t rx = spiTransfer24(0);
  data = reg::UnpackRegData(rx);
  return true;
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::applyDigitalInterface() noexcept {
  const uint16_t cfg3 =
      reg::GenCfg3Value(lanes_, length_, data_on_sdout_, format_);
  return writeRegister(RegisterBank::Common, reg::GEN_CFG3, cfg3);
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SoftwareReset() noexcept {
  if (!writeRegister(RegisterBank::Common, reg::GEN_CFG1, reg::GEN_CFG1_SW_RST)) {
    return false;
  }
  host_.DelayMs(ADS9324_CFG::RESET_DELAY_MS);
  current_bank_ = RegisterBank::Common;
  return applyDigitalInterface();
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::EnsureInitialized(bool force) noexcept {
  if (initialized_ && !force) {
    return true;
  }

  host_.DelayMs(ADS9324_CFG::POWER_ON_DELAY_MS);

  if (!SoftwareReset()) {
    ++errors_;
    return false;
  }

  if (!writeRegister(RegisterBank::Common, reg::BANK_SEL, reg::BANK_COMMON)) {
    ++errors_;
    return false;
  }

  if (!applyDigitalInterface()) {
    ++errors_;
    return false;
  }

  if (ADS9324_CFG::REQUIRE_DEVICE_ID) {
    const uint16_t id = ReadDeviceId();
    if (id != kDeviceIdAds9324) {
      ++errors_;
      return false;
    }
  }

  initialized_ = true;
  return true;
}

template <typename SpiType, typename HostType>
uint16_t ADS9324<SpiType, HostType>::ReadDeviceId() noexcept {
  uint16_t id = 0;
  if (!readRegister(RegisterBank::Common, reg::DEVICE_ID, id)) {
    return 0;
  }
  return id;
}

/**
 * PGA_CONFIG_AINxy holds two channels: even local index in bits [7:0], odd in
 * [15:8]. Bandwidth lives in PGA_BW_SEL (2 bits × local index).
 */
template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::ConfigureChannel(uint8_t channel,
                                                  const ChannelConfig& cfg) noexcept {
  if (channel >= kNumChannels) {
    return false;
  }
  const uint8_t local = reg::LocalChannel(channel);
  const RegisterBank bank = reg::BankForChannel(channel);
  const uint8_t addr = reg::PgaConfigAddr(local);

  uint16_t current = 0;
  if (!readRegister(bank, addr, current)) {
    return false;
  }

  const uint16_t nibble = reg::EncodePgaNibble(cfg);
  if ((local & 1u) == 0) {
    current = static_cast<uint16_t>((current & 0xFF00u) | (nibble & 0x00FFu));
  } else {
    current = static_cast<uint16_t>((current & 0x00FFu) | (static_cast<uint16_t>(nibble) << 8));
  }
  if (!writeRegister(bank, addr, current)) {
    return false;
  }

  uint16_t bw = 0;
  if (!readRegister(bank, reg::PGA_BW_SEL, bw)) {
    return false;
  }
  const uint8_t shift = static_cast<uint8_t>(local * 2u);
  bw = static_cast<uint16_t>((bw & static_cast<uint16_t>(~(0x3u << shift))) |
                             (static_cast<uint16_t>(static_cast<uint16_t>(cfg.bandwidth) & 0x3u)
                              << shift));
  if (!writeRegister(bank, reg::PGA_BW_SEL, bw)) {
    return false;
  }

  ch_cfg_[channel] = cfg;
  return true;
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::ConfigureAllChannels(const ChannelConfig& cfg) noexcept {
  for (uint8_t ch = 0; ch < kNumChannels; ++ch) {
    if (!ConfigureChannel(ch, cfg)) {
      return false;
    }
  }
  return true;
}

template <typename SpiType, typename HostType>
ChannelConfig ADS9324<SpiType, HostType>::GetChannelConfig(uint8_t channel) const noexcept {
  if (channel >= kNumChannels) {
    return ChannelConfig{};
  }
  return ch_cfg_[channel];
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetChannelOffset(uint8_t channel, int16_t ofs_10bit) noexcept {
  if (channel >= kNumChannels) {
    return false;
  }
  const uint16_t coded = static_cast<uint16_t>(ofs_10bit) & 0x03FFu;
  return writeRegister(reg::BankForChannel(channel), reg::OfsAddr(reg::LocalChannel(channel)),
                       coded);
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetChannelGain(uint8_t channel, uint16_t gan_14bit) noexcept {
  if (channel >= kNumChannels) {
    return false;
  }
  return writeRegister(reg::BankForChannel(channel), reg::GanAddr(reg::LocalChannel(channel)),
                       static_cast<uint16_t>(gan_14bit & 0x3FFFu));
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetDataFormat(DataFormat format) noexcept {
  format_ = format;
  return applyDigitalInterface();
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetDigitalInterface(DoutLaneMode lanes, DoutLength length,
                                                     bool data_on_sdout) noexcept {
  lanes_ = lanes;
  length_ = length;
  data_on_sdout_ = data_on_sdout;
  return applyDigitalInterface();
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetChannelCountMode(ChannelCountMode mode,
                                                     uint8_t ch_sel) noexcept {
  ch_count_mode_ = mode;
  ch_sel_ = static_cast<uint8_t>(ch_sel & 0x07u);
  return writeRegister(RegisterBank::Common, reg::ANA_CFG2,
                       reg::AnaCfg2Value(mode, ch_sel_));
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetPowerDown(bool power_down) noexcept {
  return writeRegister(RegisterBank::Common, reg::PDN_CTL,
                       power_down ? reg::PDN_CTL_DEVICE_PDN : uint16_t{0});
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::SetAlarmPin(AlarmPinMode mode, bool active_low) noexcept {
  uint16_t v = static_cast<uint16_t>(static_cast<uint16_t>(mode) & 0x0Fu);
  if (active_low) {
    v |= (1u << 4);
  }
  return writeRegister(RegisterBank::Common, reg::DRDY_ALARM_SEL, v);
}

template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::ProgramWindowComparator(uint8_t channel, int8_t high_th,
                                                         int8_t low_th, bool enable) noexcept {
  if (channel >= kNumChannels) {
    return false;
  }
  const RegisterBank bank = reg::BankForChannel(channel);
  const uint8_t local = reg::LocalChannel(channel);
  const uint16_t th = (static_cast<uint16_t>(static_cast<uint8_t>(high_th)) << 8) |
                      static_cast<uint16_t>(static_cast<uint8_t>(low_th));
  if (!writeRegister(bank, reg::DwcThAddr(local), th)) {
    return false;
  }
  uint16_t cfg = 0;
  if (!readRegister(bank, reg::DWC_CFG, cfg)) {
    return false;
  }
  const uint16_t bit = static_cast<uint16_t>(1u << local);
  if (enable) {
    cfg = static_cast<uint16_t>(cfg | bit);
  } else {
    cfg = static_cast<uint16_t>(cfg & static_cast<uint16_t>(~bit));
  }
  return writeRegister(bank, reg::DWC_CFG, cfg);
}

template <typename SpiType, typename HostType>
uint8_t ADS9324<SpiType, HostType>::activeChannelCount() const noexcept {
  switch (ch_count_mode_) {
    case ChannelCountMode::Eight: return 8;
    case ChannelCountMode::Four:  return 4;
    case ChannelCountMode::Two:   return 2;
    case ChannelCountMode::All16:
    default:                      return kNumChannels;
  }
}

/**
 * CONVST high ~1 µs then low (conversion on falling edge). If DRDY is wired,
 * poll up to DRDY_TIMEOUT_US; otherwise wait CONV_FALLBACK_US.
 */
template <typename SpiType, typename HostType>
bool ADS9324<SpiType, HostType>::pulseConvstAndWait() noexcept {
  host_.ConvstWrite(true);
  host_.DelayUs(1);
  host_.ConvstWrite(false);

  if (host_.HasDrdy()) {
    const uint32_t budget = ADS9324_CFG::DRDY_TIMEOUT_US;
    for (uint32_t i = 0; i < budget; ++i) {
      if (host_.DrdyRead()) {
        return true;
      }
      host_.DelayUs(1);
    }
    return false;
  }
  host_.DelayUs(ADS9324_CFG::CONV_FALLBACK_US);
  return true;
}

/**
 * One CS-low burst of N×2 (16-bit) or N×3 (24-bit) bytes. 24-bit mode still
 * uses the first two bytes as the 16-bit code. Channel i in the burst maps to
 * API channel i when ADC_NUM_SEL is All16.
 */
template <typename SpiType, typename HostType>
Snapshot ADS9324<SpiType, HostType>::ReadSnapshot() noexcept {
  Snapshot snap{};
  if (!initialized_) {
    snap.error = Error::NotInitialized;
    ++errors_;
    return snap;
  }
  if (!pulseConvstAndWait()) {
    snap.error = Error::Timeout;
    ++errors_;
    return snap;
  }

  const uint8_t n = activeChannelCount();
  const std::size_t bytes_per = (length_ == DoutLength::Bits24) ? 3u : 2u;
  uint8_t rx[kNumChannels * 3]{};
  uint8_t tx[kNumChannels * 3]{};
  spi_.transfer(tx, rx, static_cast<std::size_t>(n) * bytes_per);

  for (uint8_t i = 0; i < n && i < kNumChannels; ++i) {
    uint16_t raw = 0;
    if (bytes_per == 2) {
      raw = static_cast<uint16_t>((static_cast<uint16_t>(rx[i * 2]) << 8) | rx[i * 2 + 1]);
    } else {
      raw = static_cast<uint16_t>((static_cast<uint16_t>(rx[i * 3]) << 8) | rx[i * 3 + 1]);
    }
    const int16_t twos = WireWordToTwos(raw, format_);
    snap.count[i] = twos;
    snap.voltage[i] = CountToVoltage(i, twos);
    snap.valid_mask = static_cast<uint16_t>(snap.valid_mask | (uint16_t{1} << i));
  }
  ++snapshots_;
  return snap;
}

template <typename SpiType, typename HostType>
ReadResult ADS9324<SpiType, HostType>::ReadChannel(uint8_t channel) noexcept {
  ReadResult r{};
  r.channel = channel;
  if (channel >= kNumChannels) {
    r.error = Error::InvalidChannel;
    ++errors_;
    return r;
  }
  const Snapshot snap = ReadSnapshot();
  r.error = snap.error;
  if (!snap.ok() || !snap.hasChannel(channel)) {
    if (r.error == Error::Ok) {
      r.error = Error::SpiError;
    }
    return r;
  }
  r.count = snap.count[channel];
  r.raw = static_cast<uint16_t>(snap.count[channel]);
  r.voltage = snap.voltage[channel];
  return r;
}

template <typename SpiType, typename HostType>
float ADS9324<SpiType, HostType>::CountToVoltage(uint8_t channel, int16_t twos) const noexcept {
  const float fs = (channel < kNumChannels) ? FullScaleVolts(ch_cfg_[channel].range) : 5.0f;
  return TwosToVoltage(twos, fs);
}

template <typename SpiType, typename HostType>
Diagnostics ADS9324<SpiType, HostType>::GetDiagnostics() const noexcept {
  Diagnostics d{};
  d.initialized = initialized_;
  d.format = format_;
  d.lanes = lanes_;
  d.data_on_sdout = data_on_sdout_;
  d.snapshots = snapshots_;
  d.errors = errors_;
  return d;
}

}  // namespace ads9324
