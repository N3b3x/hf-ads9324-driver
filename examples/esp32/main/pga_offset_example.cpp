/**
 * @file pga_offset_example.cpp
 * @brief Per-channel PGA range/type plus offset and gain calibration
 */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../../inc/ads9324.hpp"
#include "esp32_ads9324_bus.hpp"

static const char* TAG = "ADS9324_PGA";

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

  ads9324::ChannelConfig se;
  se.type = ads9324::InputType::SingleEnded;
  se.range = ads9324::InputRange::PlusMinus10V;
  se.bandwidth = ads9324::PgaBandwidth::Wide;
  adc.ConfigureChannel(0, se);

  ads9324::ChannelConfig diff;
  diff.type = ads9324::InputType::Differential;
  diff.range = ads9324::InputRange::PlusMinus2V5;
  adc.ConfigureChannel(1, diff);

  adc.SetChannelOffset(0, 0);
  adc.SetChannelGain(0, 0);

  for (;;) {
    auto r0 = adc.ReadChannel(0);
    auto r1 = adc.ReadChannel(1);
    ESP_LOGI(TAG, "SE±10V CH0=%d %.4fV | DIFF±2.5V CH1=%d %.4fV", r0.count,
             static_cast<double>(r0.voltage), r1.count, static_cast<double>(r1.voltage));
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}
