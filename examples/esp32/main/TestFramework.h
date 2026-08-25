/**
 * @file TestFramework.h
 * @brief Shared testing framework for ESP32 comprehensive test suites
 *
 * This file provides common testing infrastructure including test result tracking,
 * execution timing, and standardized test execution macros used across all
 * comprehensive test suites.
 *
 * @author N3b3x
 * @date 2025
 * @copyright HardFOC
 * @ingroup ads9324_examples_support
 */

#pragma once

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

//=============================================================================
// GPIO14 TEST PROGRESSION INDICATOR MANAGEMENT
//=============================================================================

static bool g_test_progress_initialized = false;
static bool g_test_progress_state = false;
static constexpr gpio_num_t TEST_PROGRESS_PIN = GPIO_NUM_14;

/**
 * @ingroup ads9324_examples_support
 * @brief Initialize GPIO14 test progress indicator.
 * @return true if initialized or already initialized.
 */
inline bool init_test_progress_indicator() noexcept {
  if (g_test_progress_initialized) {
    return true;
  }

  gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << TEST_PROGRESS_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };

  esp_err_t ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    const char* TAG = "TestFramework";
    ESP_LOGE(TAG, "Failed to configure GPIO14: %s", esp_err_to_name(ret));
    return false;
  }

  gpio_set_level(TEST_PROGRESS_PIN, 0);
  g_test_progress_state = false;
  g_test_progress_initialized = true;
  return true;
}

/**
 * @ingroup ads9324_examples_support
 * @brief Toggle GPIO14 indicator and emit status log.
 */
inline void flip_test_progress_indicator() noexcept {
  if (!g_test_progress_initialized) return;
  g_test_progress_state = !g_test_progress_state;
  gpio_set_level(TEST_PROGRESS_PIN, g_test_progress_state ? 1 : 0);
  const char* TAG = "TestFramework";
  ESP_LOGI(TAG, "Test progression indicator: %s", g_test_progress_state ? "HIGH" : "LOW");
  vTaskDelay(pdMS_TO_TICKS(50));
}

/**
 * @ingroup ads9324_examples_support
 * @brief Release GPIO14 resources and reset indicator state.
 */
inline void cleanup_test_progress_indicator() noexcept {
  if (g_test_progress_initialized) {
    gpio_set_level(TEST_PROGRESS_PIN, 0);
    gpio_reset_pin(TEST_PROGRESS_PIN);
    g_test_progress_initialized = false;
    g_test_progress_state = false;
  }
}

/**
 * @ingroup ads9324_examples_support
 * @brief Blink GPIO14 multiple times to mark section boundaries.
 * @param blink_count Number of blink pulses.
 */
inline void output_section_indicator(uint8_t blink_count = 5) noexcept {
  if (!g_test_progress_initialized) return;
  for (uint8_t i = 0; i < blink_count; ++i) {
    gpio_set_level(TEST_PROGRESS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(TEST_PROGRESS_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  g_test_progress_state = false;
}

/**
 * @ingroup ads9324_examples_support
 * @brief Ensure GPIO14 indicator has been initialized.
 */
inline void ensure_gpio14_initialized() noexcept {
  if (!g_test_progress_initialized) {
    init_test_progress_indicator();
  }
}

/**
 * @brief Test execution tracking and results accumulation
 */
struct TestResults {
  int total_tests = 0;
  int passed_tests = 0;
  int failed_tests = 0;
  uint64_t total_execution_time_us = 0;

  /**
   * @brief Record one test result.
   * @param passed True if the test passed.
   * @param execution_time Execution time in microseconds.
   */
  void add_result(bool passed, uint64_t execution_time) noexcept {
    total_tests++;
    total_execution_time_us += execution_time;
    if (passed) {
      passed_tests++;
    } else {
      failed_tests++;
    }
  }

  /**
   * @brief Compute pass percentage over all executed tests.
   * @return Success percentage in [0, 100].
   */
  float get_success_percentage() const noexcept {
    return total_tests > 0 ? (static_cast<float>(passed_tests) / total_tests * 100.0f) : 0.0f;
  }

  /**
   * @brief Compute cumulative execution time in milliseconds.
   * @return Total test runtime in milliseconds.
   */
  float get_total_time_ms() const noexcept {
    return total_execution_time_us / 1000.0f;
  }
};

#define RUN_TEST(test_func)                                                                        \
  do {                                                                                             \
    ensure_gpio14_initialized();                                                                   \
    ESP_LOGI(TAG,                                                                                  \
             "\n"                                                                                  \
             "╔══════════════════════════════════════════════════════════════════════════════╗\n"  \
             "║ Running: " #test_func "                                                     \n"    \
             "╚══════════════════════════════════════════════════════════════════════════════╝");  \
    uint64_t start_time = esp_timer_get_time();                                                    \
    bool result = test_func();                                                                     \
    uint64_t end_time = esp_timer_get_time();                                                      \
    uint64_t execution_time = end_time - start_time;                                               \
    g_test_results.add_result(result, execution_time);                                             \
    if (result) {                                                                                  \
      ESP_LOGI(TAG, "[SUCCESS] PASSED: " #test_func " (%.2f ms)", execution_time / 1000.0);        \
    } else {                                                                                       \
      ESP_LOGE(TAG, "[FAILED] FAILED: " #test_func " (%.2f ms)", execution_time / 1000.0);         \
    }                                                                                              \
    vTaskDelay(pdMS_TO_TICKS(100));                                                                \
  } while (0)

struct TestTaskContext {
  const char* test_name;
  bool (*test_func)() noexcept;
  TestResults* results;
  const char* tag;
  SemaphoreHandle_t completion_semaphore;
};

/**
 * @ingroup ads9324_examples_support
 * @brief FreeRTOS task entry point for executing one test case.
 * @param param Pointer to TestTaskContext.
 */
inline void test_task_trampoline(void* param) {
  TestTaskContext* ctx = static_cast<TestTaskContext*>(param);
  ESP_LOGI(ctx->tag,
           "\n╔══════════════════════════════════════════════════════════════════════════════╗\n"
           "║ Running (task): %s                                                            \n"
           "╚══════════════════════════════════════════════════════════════════════════════╝",
           ctx->test_name);
  uint64_t start_time = esp_timer_get_time();
  bool result = ctx->test_func();
  uint64_t end_time = esp_timer_get_time();
  uint64_t execution_time = end_time - start_time;
  ctx->results->add_result(result, execution_time);
  if (result) {
    ESP_LOGI(ctx->tag, "[SUCCESS] PASSED (task): %s (%.2f ms)", ctx->test_name,
             execution_time / 1000.0);
  } else {
    ESP_LOGE(ctx->tag, "[FAILED] FAILED (task): %s (%.2f ms)", ctx->test_name,
             execution_time / 1000.0);
  }
  if (ctx->completion_semaphore != nullptr) {
    xSemaphoreGive(ctx->completion_semaphore);
  }
  vTaskDelete(nullptr);
}

#define RUN_TEST_IN_TASK(name, func, stack_size_bytes, priority)                                   \
  do {                                                                                             \
    ensure_gpio14_initialized();                                                                   \
    static TestTaskContext ctx;                                                                     \
    ctx.test_name = name;                                                                          \
    ctx.test_func = func;                                                                          \
    ctx.results = &g_test_results;                                                                 \
    ctx.tag = TAG;                                                                                 \
    ctx.completion_semaphore = xSemaphoreCreateBinary();                                           \
    if (ctx.completion_semaphore == nullptr) {                                                     \
      ESP_LOGE(TAG, "Failed to create semaphore for test: %s", name);                              \
      RUN_TEST(func);                                                                              \
    } else {                                                                                       \
      BaseType_t created =                                                                         \
          xTaskCreate(test_task_trampoline, name, (stack_size_bytes) / sizeof(StackType_t), &ctx,  \
                      (priority), nullptr);                                                        \
      if (created != pdPASS) {                                                                     \
        ESP_LOGE(TAG, "Failed to create test task: %s", name);                                     \
        vSemaphoreDelete(ctx.completion_semaphore);                                                \
        RUN_TEST(func);                                                                            \
      } else {                                                                                     \
        if (xSemaphoreTake(ctx.completion_semaphore, pdMS_TO_TICKS(30000)) == pdTRUE) {            \
          ESP_LOGI(TAG, "Test task completed: %s", name);                                          \
        } else {                                                                                   \
          ESP_LOGW(TAG, "Test task timeout: %s", name);                                            \
        }                                                                                          \
        vSemaphoreDelete(ctx.completion_semaphore);                                                \
        vTaskDelay(pdMS_TO_TICKS(100));                                                            \
      }                                                                                            \
    }                                                                                              \
  } while (0)

inline void print_test_summary(const TestResults& test_results, const char* test_suite_name,
                               const char* tag) noexcept {
  ensure_gpio14_initialized();
  ESP_LOGI(tag, "\n=== %s TEST SUMMARY ===", test_suite_name);
  ESP_LOGI(tag, "Total: %d, Passed: %d, Failed: %d, Success: %.2f%%, Time: %.2f ms",
           test_results.total_tests, test_results.passed_tests, test_results.failed_tests,
           test_results.get_success_percentage(), test_results.get_total_time_ms());

  if (test_results.failed_tests == 0) {
    ESP_LOGI(tag, "[SUCCESS] ALL %s TESTS PASSED!", test_suite_name);
  } else {
    ESP_LOGE(tag, "[FAILED] Some tests failed. Review the results above.");
  }
}

/**
 * @ingroup ads9324_examples_support
 * @brief Print section-configuration help banner.
 * @param tag ESP log tag.
 * @param test_suite_name Human-readable test suite name.
 */
inline void print_test_section_status(const char* tag, const char* test_suite_name) noexcept {
  ensure_gpio14_initialized();
  ESP_LOGI(tag, "\n");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
  ESP_LOGI(tag, "║                 %s TEST SECTION CONFIGURATION                                ║",
           test_suite_name);
  ESP_LOGI(tag, "╚══════════════════════════════════════════════════════════════════════════════╝");
  ESP_LOGI(tag, "To modify test sections, edit the defines at the top of your test file");
  ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
}

/**
 * @ingroup ads9324_examples_support
 * @brief Print a section header for enabled/disabled test groups.
 * @param tag ESP log tag.
 * @param section_name Section label.
 * @param enabled Whether the section is enabled.
 */
inline void print_test_section_header(const char* tag, const char* section_name,
                                      bool enabled = true) noexcept {
  if (enabled) {
    ESP_LOGI(tag, "\n");
    ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(tag, "║                              %s                                              ",
             section_name);
    ESP_LOGI(tag, "╠══════════════════════════════════════════════════════════════════════════════╣");
  } else {
    ESP_LOGI(tag, "\n");
    ESP_LOGI(tag, "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(tag, "║                         %s (DISABLED)                                  ",
             section_name);
    ESP_LOGI(tag, "╚══════════════════════════════════════════════════════════════════════════════╝");
  }
}
