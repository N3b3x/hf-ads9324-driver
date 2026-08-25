/**
 * @file basic_snapshot_example.cpp
 * @brief Minimal ADS9324 bring-up: init, PGA, CONVST snapshot on ESP32-C6
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../../inc/ads9324.hpp"
#include "esp32_ads9324_bus.hpp"

static const char* TAG = "ADS9324_Basic";

extern "C" void app_main(void) {
  auto bus = CreateEsp32Ads9324Bus();
  if (!bus || !bus->initialize()) {
    ESP_LOGE(TAG, "bus init failed");
    return;
  }
  ads9324::ADS9324<Esp32Ads9324Bus, Esp32Ads9324Bus> adc(*bus, *bus);
  if (!adc.EnsureInitialized()) {
    ESP_LOGE(TAG, "init failed");
    return;
  }
  ads9324::ChannelConfig cfg;
  cfg.type = ads9324::InputType::Differential;
  cfg.range = ads9324::InputRange::PlusMinus5V;
  adc.ConfigureAllChannels(cfg);

  while (true) {
    auto snap = adc.ReadSnapshot();
    if (snap.ok()) {
      ESP_LOGI(TAG, "CH0=%d (%.4f V)  CH1=%d (%.4f V)", snap.count[0],
               static_cast<double>(snap.voltage[0]), snap.count[1],
               static_cast<double>(snap.voltage[1]));
    } else {
      ESP_LOGW(TAG, "snapshot error %u", static_cast<unsigned>(snap.error));
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
