/**
 * @file driver_integration_test.cpp
 * @brief ESP32-C6 integration test suite for the ADS9324 driver
 * @ingroup ads9324_examples_tests
 *
 * Requires ADS9324 silicon on the pins in esp32_ads9324_test_config.hpp.
 * DEVICE_ID, PGA programming, offset/gain, CONVST snapshot, and DWC are covered.
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>
#include <memory>
#include <stdio.h>

#include "../../../inc/ads9324.hpp"
#include "TestFramework.h"
#include "esp32_ads9324_bus.hpp"
#include "esp32_ads9324_test_config.hpp"

#define ENABLE_INITIALIZATION_TESTS 1
#define ENABLE_PGA_TESTS            1
#define ENABLE_SNAPSHOT_TESTS       1
#define ENABLE_CAL_TESTS            1
#define ENABLE_POWER_DOWN_TESTS     1

static const char* TAG = "ADS9324_Test";
static TestResults g_test_results;
static std::unique_ptr<Esp32Ads9324Bus> g_bus;
static ads9324::ADS9324<Esp32Ads9324Bus, Esp32Ads9324Bus>* g_adc = nullptr;

static bool test_spi_bus_initialization() noexcept {
  return g_bus && g_bus->isInitialized();
}

static bool test_driver_initialization() noexcept {
  return g_adc && g_adc->IsInitialized();
}

static bool test_device_id() noexcept {
  const uint16_t id = g_adc->ReadDeviceId();
  ESP_LOGI(TAG, "DEVICE_ID=0x%04X (expect 0x%04X)", id, ADS9324_TestConfig::ADCSpecs::DEVICE_ID);
  return id == ADS9324_TestConfig::ADCSpecs::DEVICE_ID;
}

static bool test_configure_single_ended_10v() noexcept {
  ads9324::ChannelConfig cfg;
  cfg.type = ads9324::InputType::SingleEnded;
  cfg.range = ads9324::InputRange::PlusMinus10V;
  cfg.bandwidth = ads9324::PgaBandwidth::Low;
  return g_adc->ConfigureChannel(0, cfg) &&
         g_adc->GetChannelConfig(0).range == ads9324::InputRange::PlusMinus10V;
}

static bool test_configure_all_differential() noexcept {
  ads9324::ChannelConfig cfg;
  cfg.type = ads9324::InputType::Differential;
  cfg.range = ads9324::InputRange::PlusMinus5V;
  return g_adc->ConfigureAllChannels(cfg);
}

static bool test_snapshot() noexcept {
  auto snap = g_adc->ReadSnapshot();
  ESP_LOGI(TAG, "snapshot err=%u mask=0x%04X CH0=%d (%.4f V)", static_cast<unsigned>(snap.error),
           snap.valid_mask, snap.count[0], static_cast<double>(snap.voltage[0]));
  return snap.ok() && snap.hasChannel(0) && snap.hasChannel(15);
}

static bool test_read_channel_0() noexcept {
  auto r = g_adc->ReadChannel(0);
  ESP_LOGI(TAG, "CH0 count=%d V=%.4f err=%u", r.count, static_cast<double>(r.voltage),
           static_cast<unsigned>(r.error));
  return r.ok();
}

static bool test_invalid_channel() noexcept {
  return g_adc->ReadChannel(16).error == ads9324::Error::InvalidChannel;
}

static bool test_offset_gain_program() noexcept {
  bool ok = g_adc->SetChannelOffset(0, 0) && g_adc->SetChannelGain(0, 0);
  ok = ok && g_adc->SetChannelOffset(15, 1) && g_adc->SetChannelGain(15, 0x0CCD);
  return ok;
}

static bool test_window_comparator() noexcept {
  return g_adc->ProgramWindowComparator(0, 64, -64, true) &&
         g_adc->ProgramWindowComparator(0, 127, -128, false);
}

static bool test_power_down_cycle() noexcept {
  if (!g_adc->SetPowerDown(true)) {
    return false;
  }
  vTaskDelay(pdMS_TO_TICKS(5));
  return g_adc->SetPowerDown(false);
}

static bool test_voltage_math() noexcept {
  const float z = ads9324::TwosToVoltage(0, 5.0f);
  const float p = ads9324::TwosToVoltage(32767, 5.0f);
  return std::fabs(z) < 1e-6f && p > 4.9f;
}

extern "C" void app_main(void) {
  init_test_progress_indicator();
  print_test_section_status(TAG, "ADS9324");

  g_bus = CreateEsp32Ads9324Bus();
  if (!g_bus || !g_bus->initialize()) {
    ESP_LOGE(TAG, "Bus init failed");
    return;
  }
  static ads9324::ADS9324<Esp32Ads9324Bus, Esp32Ads9324Bus> adc(*g_bus, *g_bus);
  g_adc = &adc;
  if (!g_adc->EnsureInitialized()) {
    ESP_LOGE(TAG, "EnsureInitialized failed (check wiring / 30 ms analog rail)");
  }

#if ENABLE_INITIALIZATION_TESTS
  print_test_section_header(TAG, "INITIALIZATION");
  RUN_TEST(test_spi_bus_initialization);
  RUN_TEST(test_driver_initialization);
  RUN_TEST(test_device_id);
#endif
#if ENABLE_PGA_TESTS
  print_test_section_header(TAG, "PGA");
  RUN_TEST(test_configure_single_ended_10v);
  RUN_TEST(test_configure_all_differential);
#endif
#if ENABLE_SNAPSHOT_TESTS
  print_test_section_header(TAG, "SNAPSHOT");
  RUN_TEST(test_snapshot);
  RUN_TEST(test_read_channel_0);
  RUN_TEST(test_invalid_channel);
  RUN_TEST(test_voltage_math);
#endif
#if ENABLE_CAL_TESTS
  print_test_section_header(TAG, "OFFSET_GAIN_DWC");
  RUN_TEST(test_offset_gain_program);
  RUN_TEST(test_window_comparator);
#endif
#if ENABLE_POWER_DOWN_TESTS
  print_test_section_header(TAG, "POWER_DOWN");
  RUN_TEST(test_power_down_cycle);
#endif

  print_test_summary(g_test_results, "ADS9324", TAG);
  cleanup_test_progress_indicator();
}
