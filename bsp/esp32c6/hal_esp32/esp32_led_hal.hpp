/**
 * @file esp32_led_hal.hpp
 * @brief LED HAL implementation for ESP32 (ESP-IDF)
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#ifndef ESP32_LED_HAL_HPP
#define ESP32_LED_HAL_HPP

#include "v4std/sys_led.hpp"

namespace v4rtos
{

/**
 * @brief LED HAL implementation for ESP32 using ESP-IDF GPIO API
 */
class Esp32LedHal : public v4std::LedHal
{
 public:
  bool set_led(uint32_t handle, bool state, bool active_low) override;
  bool get_led(uint32_t handle, bool active_low) override;
};

}  // namespace v4rtos

#endif  // ESP32_LED_HAL_HPP
