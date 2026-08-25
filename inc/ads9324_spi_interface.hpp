/**
 * @file ads9324_spi_interface.hpp
 * @brief CRTP SPI transport for ADS9324 (24-bit config frames + conversion burst)
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_transport
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ads9324 {

/**
 * @defgroup ads9324_transport Transport Abstraction
 * @ingroup ads9324_driver
 */

/**
 * @ingroup ads9324_transport
 * @brief CRTP SPI contract. CS is owned by the implementation (one CS per call).
 *
 * Configuration traffic is 3 bytes. Conversion readout is a single CS-low burst
 * of `num_channels * 2` bytes in 16-bit 1-lane SDOUT mode.
 */
template <typename Derived>
class SpiInterface {
public:
  void transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    return static_cast<Derived*>(this)->transfer(tx, rx, len);
  }

  SpiInterface(const SpiInterface&) = delete;
  SpiInterface& operator=(const SpiInterface&) = delete;

protected:
  SpiInterface() = default;
  SpiInterface(SpiInterface&&) = default;
  SpiInterface& operator=(SpiInterface&&) = default;
  ~SpiInterface() = default;
};

/**
 * @ingroup ads9324_transport
 * @brief CONVST / DRDY / delay pins. DRDY may be unused (HasDrdy() == false).
 */
template <typename Derived>
class HostInterface {
public:
  void ConvstWrite(bool high) { static_cast<Derived*>(this)->ConvstWrite(high); }
  [[nodiscard]] bool DrdyRead() { return static_cast<Derived*>(this)->DrdyRead(); }
  [[nodiscard]] bool HasDrdy() const { return static_cast<const Derived*>(this)->HasDrdy(); }
  void DelayUs(uint32_t us) { static_cast<Derived*>(this)->DelayUs(us); }
  void DelayMs(uint32_t ms) { static_cast<Derived*>(this)->DelayMs(ms); }

  HostInterface(const HostInterface&) = delete;
  HostInterface& operator=(const HostInterface&) = delete;

protected:
  HostInterface() = default;
  HostInterface(HostInterface&&) = default;
  HostInterface& operator=(HostInterface&&) = default;
  ~HostInterface() = default;
};

}  // namespace ads9324
