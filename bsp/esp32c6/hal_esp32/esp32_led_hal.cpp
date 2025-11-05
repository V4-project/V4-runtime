/**
 * @file esp32_led_hal.cpp
 * @brief LED HAL implementation for ESP32 (ESP-IDF)
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include "esp32_led_hal.hpp"

#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "esp32_led_hal";

namespace v4rtos
{

bool Esp32LedHal::set_led(uint32_t handle, bool state, bool active_low)
{
  gpio_num_t gpio = static_cast<gpio_num_t>(handle);

  // Apply active-low logic
  uint32_t level = state ? 1 : 0;
  if (active_low)
  {
    level = !level;
  }

  esp_err_t err = gpio_set_level(gpio, level);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to set GPIO%d: %d", gpio, err);
    return false;
  }

  ESP_LOGD(TAG, "LED GPIO%d set to %s (logical=%s, active_low=%s)", gpio,
           level ? "HIGH" : "LOW", state ? "ON" : "OFF", active_low ? "yes" : "no");

  return true;
}

bool Esp32LedHal::get_led(uint32_t handle, bool active_low)
{
  gpio_num_t gpio = static_cast<gpio_num_t>(handle);

  int level = gpio_get_level(gpio);

  // Apply active-low logic when reading
  bool logical_state = (level != 0);
  if (active_low)
  {
    logical_state = !logical_state;
  }

  ESP_LOGD(TAG, "LED GPIO%d read as %s (physical=%s, active_low=%s)", gpio,
           logical_state ? "ON" : "OFF", level ? "HIGH" : "LOW",
           active_low ? "yes" : "no");

  return logical_state;
}

}  // namespace v4rtos
