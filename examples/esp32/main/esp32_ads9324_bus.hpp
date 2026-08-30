/**
 * @file esp32_ads9324_bus.hpp
 * @brief ESP32-C6 SPI + CONVST/DRDY transport for the ADS9324 driver
 * @ingroup ads9324_examples_support
 *
 * @details
 * Implements both CRTP contracts on one type so examples can pass the same
 * object as SpiType and HostType. CS is handled by the ESP-IDF SPI device
 * driver (one transaction = one CS-low window).
 *
 * @see esp32_ads9324_test_config.hpp
 */
#pragma once

#include "../../../inc/ads9324_spi_interface.hpp"
#include "esp32_ads9324_test_config.hpp"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>
#include <cstring>
#include <memory>

/**
 * @ingroup ads9324_examples_support
 * @brief ESP-IDF SPI2 + GPIO backend for ADS9324 examples.
 */
class Esp32Ads9324Bus : public ads9324::SpiInterface<Esp32Ads9324Bus>,
                        public ads9324::HostInterface<Esp32Ads9324Bus> {
public:
  /** @brief Pin and SPI-host settings (filled from ADS9324_TestConfig). */
  struct Config {
    spi_host_device_t host;  ///< SPI2_HOST on ESP32-C6.
    gpio_num_t miso_pin;     ///< SDOUT.
    gpio_num_t mosi_pin;     ///< SDI.
    gpio_num_t sclk_pin;     ///< SCLK.
    gpio_num_t cs_pin;       ///< Active-low CS.
    gpio_num_t convst_pin;   ///< CONVST output.
    gpio_num_t drdy_pin;     ///< DRDY input (ignored when @ref use_drdy is false).
    bool use_drdy;           ///< false → timed CONVST fallback.
    uint32_t frequency;      ///< Hz (example default 8 MHz).
    uint8_t mode;            ///< 0 = mode 0.
    uint8_t queue_size;
    uint8_t cs_ena_pretrans;
    uint8_t cs_ena_posttrans;
  };

  /** @brief Store config; call @ref initialize before @ref transfer. */
  explicit Esp32Ads9324Bus(const Config& config) : config_(config) {}
  ~Esp32Ads9324Bus() { deinitialize(); }

  void transfer(const uint8_t* tx, uint8_t* rx, std::size_t len) {
    if (!initialized_ || spi_device_ == nullptr) {
      ESP_LOGE(TAG, "SPI bus not initialized");
      return;
    }
    spi_transaction_t trans = {};
    trans.length = len * 8;
    trans.tx_buffer = tx;
    trans.rx_buffer = rx;
    esp_err_t ret = spi_device_transmit(spi_device_, &trans);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "SPI transfer failed: %s", esp_err_to_name(ret));
    }
#if ESP32_ADS9324_ENABLE_DETAILED_SPI_LOGGING
    if (len >= 3 && tx != nullptr && rx != nullptr) {
      ESP_LOGD(TAG, "SPI 24b TX=%02X%02X%02X RX=%02X%02X%02X", tx[0], tx[1], tx[2], rx[0], rx[1],
               rx[2]);
    }
#endif
  }

  void ConvstWrite(bool high) { gpio_set_level(config_.convst_pin, high ? 1 : 0); }

  bool DrdyRead() {
    if (!config_.use_drdy) {
      return true;
    }
    return gpio_get_level(config_.drdy_pin) != 0;
  }

  bool HasDrdy() const { return config_.use_drdy; }

  void DelayUs(uint32_t us) { esp_rom_delay_us(us); }

  void DelayMs(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms == 0 ? 1 : ms)); }

  bool initialize() {
    if (initialized_) {
      return true;
    }

    gpio_config_t convst_cfg = {};
    convst_cfg.pin_bit_mask = 1ULL << static_cast<unsigned>(config_.convst_pin);
    convst_cfg.mode = GPIO_MODE_OUTPUT;
    convst_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    convst_cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
    convst_cfg.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&convst_cfg) != ESP_OK) {
      return false;
    }
    gpio_set_level(config_.convst_pin, 0);

    if (config_.use_drdy) {
      gpio_config_t drdy_cfg = {};
      drdy_cfg.pin_bit_mask = 1ULL << static_cast<unsigned>(config_.drdy_pin);
      drdy_cfg.mode = GPIO_MODE_INPUT;
      drdy_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
      drdy_cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
      drdy_cfg.intr_type = GPIO_INTR_DISABLE;
      if (gpio_config(&drdy_cfg) != ESP_OK) {
        return false;
      }
    }

    spi_bus_config_t bus_cfg = {};
    bus_cfg.miso_io_num = config_.miso_pin;
    bus_cfg.mosi_io_num = config_.mosi_pin;
    bus_cfg.sclk_io_num = config_.sclk_pin;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 128;

    esp_err_t ret = spi_bus_initialize(config_.host, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
      return false;
    }
    bus_initialized_ = true;

    spi_device_interface_config_t dev_cfg = {};
    dev_cfg.clock_speed_hz = static_cast<int>(config_.frequency);
    dev_cfg.mode = config_.mode;
    dev_cfg.spics_io_num = config_.cs_pin;
    dev_cfg.queue_size = config_.queue_size;
    dev_cfg.cs_ena_pretrans = config_.cs_ena_pretrans;
    dev_cfg.cs_ena_posttrans = config_.cs_ena_posttrans;

    ret = spi_bus_add_device(config_.host, &dev_cfg, &spi_device_);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "SPI device add failed: %s", esp_err_to_name(ret));
      spi_bus_free(config_.host);
      bus_initialized_ = false;
      return false;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "ADS9324 bus ready: SPI %lu Hz CS=GPIO%d CONVST=GPIO%d DRDY=%s",
             static_cast<unsigned long>(config_.frequency), static_cast<int>(config_.cs_pin),
             static_cast<int>(config_.convst_pin), config_.use_drdy ? "yes" : "timed");
    return true;
  }

  void deinitialize() {
    if (spi_device_) {
      spi_bus_remove_device(spi_device_);
      spi_device_ = nullptr;
    }
    if (bus_initialized_) {
      spi_bus_free(config_.host);
      bus_initialized_ = false;
    }
    initialized_ = false;
  }

  bool isInitialized() const { return initialized_; }

private:
  static constexpr const char* TAG = "Esp32Ads9324Bus";
  Config config_;
  spi_device_handle_t spi_device_ = nullptr;
  bool initialized_ = false;
  bool bus_initialized_ = false;
};

inline std::unique_ptr<Esp32Ads9324Bus> CreateEsp32Ads9324Bus() {
  Esp32Ads9324Bus::Config config{};
  config.host = static_cast<spi_host_device_t>(ADS9324_TestConfig::SPIParams::SPI_HOST_ID);
  config.miso_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::SPIPins::MISO);
  config.mosi_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::SPIPins::MOSI);
  config.sclk_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::SPIPins::SCLK);
  config.cs_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::SPIPins::CS);
  config.convst_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::GpioPins::CONVST);
  config.drdy_pin = static_cast<gpio_num_t>(ADS9324_TestConfig::GpioPins::DRDY);
  config.use_drdy = ADS9324_TestConfig::GpioPins::USE_DRDY;
  config.frequency = ADS9324_TestConfig::SPIParams::FREQUENCY;
  config.mode = ADS9324_TestConfig::SPIParams::MODE;
  config.queue_size = ADS9324_TestConfig::SPIParams::QUEUE_SIZE;
  config.cs_ena_pretrans = ADS9324_TestConfig::SPIParams::CS_ENA_PRETRANS;
  config.cs_ena_posttrans = ADS9324_TestConfig::SPIParams::CS_ENA_POSTTRANS;
  return std::make_unique<Esp32Ads9324Bus>(config);
}
