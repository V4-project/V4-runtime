/**
 * @file peripherals.h
 * @brief M5Stack NanoC6 Peripheral Helper Functions
 *
 * This header provides convenient helper functions for controlling
 * M5Stack NanoC6 board peripherals (LED, Button, etc.).
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#ifndef BOARD_NANOC6_PERIPHERALS_H
#define BOARD_NANOC6_PERIPHERALS_H

#include "board.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* ========================================================================= */
  /* LED Control Helpers                                                       */
  /* ========================================================================= */

  /**
   * @brief Turn on the built-in LED
   *
   * Pin: GPIO7 (active high)
   */
  static inline void board_led_on(void)
  {
    gpio_set_level(LED_PIN, LED_ACTIVE_HIGH ? 1 : 0);
  }

  /**
   * @brief Turn off the built-in LED
   *
   * Pin: GPIO7 (active high)
   */
  static inline void board_led_off(void)
  {
    gpio_set_level(LED_PIN, LED_ACTIVE_HIGH ? 0 : 1);
  }

  /**
   * @brief Toggle the built-in LED
   *
   * Pin: GPIO7 (active high)
   */
  static inline void board_led_toggle(void)
  {
    int level = gpio_get_level(LED_PIN);
    gpio_set_level(LED_PIN, !level);
  }

  /**
   * @brief Set LED to a specific state
   *
   * @param on true to turn LED on, false to turn off
   *
   * Pin: GPIO7 (active high)
   */
  static inline void board_led_set(bool on)
  {
    if (on)
    {
      board_led_on();
    }
    else
    {
      board_led_off();
    }
  }

  /* ========================================================================= */
  /* Button Helpers                                                            */
  /* ========================================================================= */

  /**
   * @brief Read button state
   *
   * @return true if button is pressed, false otherwise
   *
   * Pin: GPIO9 (active low with internal pullup)
   */
  static inline bool board_button_read(void)
  {
    int level = gpio_get_level(BUTTON_PIN);
    return BUTTON_ACTIVE_LOW ? (level == 0) : (level != 0);
  }

  /* ========================================================================= */
  /* GPIO Initialization Helpers                                               */
  /* ========================================================================= */

  /**
   * @brief Initialize LED GPIO
   *
   * Configures GPIO7 as output for LED control.
   * Must be called before using LED functions.
   *
   * @return ESP_OK on success, error code otherwise
   */
  static inline esp_err_t board_led_init(void)
  {
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&led_conf);
  }

  /**
   * @brief Initialize button GPIO
   *
   * Configures GPIO9 as input with internal pullup for button.
   * Must be called before reading button state.
   *
   * @return ESP_OK on success, error code otherwise
   */
  static inline esp_err_t board_button_init(void)
  {
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    return gpio_config(&btn_conf);
  }

  /**
   * @brief Initialize RGB LED GPIO
   *
   * Configures GPIO8 as output for WS2812 RGB LED.
   * Note: Actual RGB LED control requires additional WS2812 driver.
   *
   * @return ESP_OK on success, error code otherwise
   */
  static inline esp_err_t board_rgb_led_init(void)
  {
    esp_err_t ret;

    // Configure RGB LED power enable pin (GPIO19)
    gpio_config_t enable_conf = {
        .pin_bit_mask = (1ULL << RGB_LED_ENABLE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&enable_conf);
    if (ret != ESP_OK)
    {
      return ret;
    }

    // Enable LED power supply (set GPIO19 HIGH)
    gpio_set_level(RGB_LED_ENABLE_PIN, 1);

    // Configure RGB LED data pin (GPIO20)
    gpio_config_t rgb_conf = {
        .pin_bit_mask = (1ULL << RGB_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&rgb_conf);
    if (ret != ESP_OK)
    {
      return ret;
    }

    // Set initial LOW state
    gpio_set_level(RGB_LED_PIN, 0);

    return ESP_OK;
  }

  /**
   * @brief Initialize all board peripherals
   *
   * Convenience function that initializes:
   * - LED (GPIO7)
   * - Button (GPIO9)
   * - RGB LED (GPIO8)
   *
   * @return ESP_OK if all initializations succeed, error code otherwise
   */
  static inline esp_err_t board_peripherals_init(void)
  {
    esp_err_t ret;

    ret = board_led_init();
    if (ret != ESP_OK)
      return ret;

    ret = board_button_init();
    if (ret != ESP_OK)
      return ret;

    ret = board_rgb_led_init();
    if (ret != ESP_OK)
      return ret;

    // Turn off LED initially
    board_led_off();

    return ESP_OK;
  }

#ifdef __cplusplus
}
#endif

#endif /* BOARD_NANOC6_PERIPHERALS_H */
