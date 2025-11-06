/**
 * @file esp32_rgb_led.h
 * @brief WS2812 RGB LED driver for ESP32
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#ifndef ESP32_RGB_LED_H
#define ESP32_RGB_LED_H

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initialize WS2812 RGB LED
   *
   * @param gpio GPIO pin number
   * @param led_count Number of LEDs in the strip
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t esp32_rgb_led_init(uint8_t gpio, uint16_t led_count);

  /**
   * @brief Set RGB LED color
   *
   * @param index LED index (0-based)
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t esp32_rgb_led_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b);

  /**
   * @brief Clear RGB LED (turn off)
   *
   * @param index LED index (0-based)
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t esp32_rgb_led_clear(uint16_t index);

  /**
   * @brief Clear all RGB LEDs
   *
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t esp32_rgb_led_clear_all(void);

  /**
   * @brief Apply buffered changes to RGB LEDs
   *
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t esp32_rgb_led_refresh(void);

  /**
   * @brief Deinitialize RGB LED driver
   */
  void esp32_rgb_led_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_RGB_LED_H */
