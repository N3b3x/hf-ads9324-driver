/**
 * @file ads9324_spi_interface.hpp
 * @brief CRTP SPI and host-pin contracts for ADS9324
 * @copyright Copyright (c) 2026 HardFOC. All rights reserved.
 * @ingroup ads9324_transport
 *
 * @details
 * The silicon uses one SPI port for **24-bit configuration frames** (address +
 * data) and, in the driver's default 1-lane SDOUT mode, the same SDI/SCLK/CS
 * plus SDOUT for **conversion bursts**. CONVST and optional DRDY are GPIO,
 * not SPI.
 *
 * Implement both CRTP bases on the platform (see
 * `examples/esp32/main/esp32_ads9324_bus.hpp`).
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace ads9324 {

/**
 * @defgroup ads9324_transport Transport Abstraction
 * @ingroup ads9324_driver
 * @brief CRTP contracts implemented by the platform SPI and GPIO backends.
 * @{
 */

/**
 * @ingroup ads9324_transport
 * @brief CRTP SPI contract. CS is owned by the implementation (one CS per call).
 *
 * Configuration traffic is always 3 bytes, MSB first, SPI mode 0 (CPOL=0,
 * CPHA=0), latched on SCLK rising and decoded on CS rising (SBASB22 §7.5).
 * Conversion readout is a **single CS-low burst** of
 * `num_channels * 2` bytes in 16-bit 1-lane SDOUT mode (32 bytes for 16 ch).
 *
 * @tparam Derived Platform class that implements @ref transfer.
 *
 * @note The driver does not toggle CS. Wrap each @p transfer in CS-low for
 *       the entire @p len — do not deassert between bytes of one frame.
 * @warning Do not share this CS with another device on the same call.
 */
template <typename Derived>
class SpiInterface {
public:
  /**
   * @brief Full-duplex SPI transfer with CS asserted around the whole call.
   * @param tx Transmit buffer of @p len bytes, or `nullptr` to clock zeros
   *           (conversion readout).
   * @param rx Receive buffer of @p len bytes, or `nullptr` to discard MISO
   *           (config writes).
   * @param len Byte count. Config frames use 3. Conversion bursts use
   *            `N * 2` (16-bit) or `N * 3` (24-bit DOUT_LENGTH).
   */
  void transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    return static_cast<Derived*>(this)->transfer(tx, rx, len);
  }

  SpiInterface(const SpiInterface&) = delete;
  SpiInterface& operator=(const SpiInterface&) = delete;

protected:
  /** @brief Protected constructor — instantiate the derived type only. */
  SpiInterface() = default;
  SpiInterface(SpiInterface&&) = default;
  SpiInterface& operator=(SpiInterface&&) = default;
  ~SpiInterface() = default;
};

/**
 * @ingroup ads9324_transport
 * @brief CONVST / DRDY / delay pins. DRDY may be unused (`HasDrdy() == false`).
 *
 * @tparam Derived Platform class that implements the five methods below.
 *
 * CONVST is **active-high**: a high pulse then low starts conversion on the
 * **falling** edge (SBASB22; pulse width ≥ 50 ns). Map host GPIO “active”
 * to CONVST high.
 *
 * When @ref HasDrdy returns false, the driver waits
 * @ref ADS9324_CFG::CONV_FALLBACK_US after CONVST instead of polling.
 */
template <typename Derived>
class HostInterface {
public:
  /**
   * @brief Drive CONVST.
   * @param high true = CONVST high, false = CONVST low.
   */
  void ConvstWrite(bool high) { static_cast<Derived*>(this)->ConvstWrite(high); }

  /**
   * @brief Sample DRDY/ALARM (active meaning is "conversion complete" in
   *        default @ref AlarmPinMode::Drdy).
   * @return true if DRDY is asserted (or if the pin is not wired — see
   *         @ref HasDrdy).
   */
  [[nodiscard]] bool DrdyRead() { return static_cast<Derived*>(this)->DrdyRead(); }

  /**
   * @brief Whether a DRDY GPIO is connected.
   * @return false to use the timed CONVST fallback.
   */
  [[nodiscard]] bool HasDrdy() const { return static_cast<const Derived*>(this)->HasDrdy(); }

  /**
   * @brief Busy-wait or OS delay in microseconds.
   * @param us Delay duration.
   */
  void DelayUs(uint32_t us) { static_cast<Derived*>(this)->DelayUs(us); }

  /**
   * @brief Delay in milliseconds (power-on and reset settle).
   * @param ms Delay duration.
   */
  void DelayMs(uint32_t ms) { static_cast<Derived*>(this)->DelayMs(ms); }

  HostInterface(const HostInterface&) = delete;
  HostInterface& operator=(const HostInterface&) = delete;

protected:
  /** @brief Protected constructor — instantiate the derived type only. */
  HostInterface() = default;
  HostInterface(HostInterface&&) = default;
  HostInterface& operator=(HostInterface&&) = default;
  ~HostInterface() = default;
};

/** @} */

}  // namespace ads9324
