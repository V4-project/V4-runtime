/**
 * @file board.h
 * @brief M5Stack NanoC6 Board Configuration
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#ifndef BOARD_NANOC6_H
#define BOARD_NANOC6_H

#include <stdint.h>

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /* ========================================================================
   * Board Identification
   * ======================================================================== */

#define BOARD_NAME "M5Stack NanoC6"
#define BOARD_VARIANT "M5NANOC6"
#define BOARD_MCU "ESP32-C6"
#define BOARD_VENDOR "M5Stack"

/* ========================================================================
 * GPIO Pin Definitions
 * ======================================================================== */

/** Built-in LED (GPIO7) */
#define LED_PIN GPIO_NUM_7
#define LED_ACTIVE_HIGH 1

/** RGB LED / NeoPixel (GPIO8, WS2812) */
#define RGB_LED_PIN GPIO_NUM_8
#define RGB_LED_COUNT 1

/** Button (GPIO9, active low with internal pullup) */
#define BUTTON_PIN GPIO_NUM_9
#define BUTTON_ACTIVE_LOW 1

  /* ========================================================================
   * I2C Configuration
   * ======================================================================== */

#define I2C_SDA_PIN GPIO_NUM_1
#define I2C_SCL_PIN GPIO_NUM_2
#define I2C_FREQ_HZ 100000 /* 100 kHz */

/* ========================================================================
 * UART Configuration
 * ======================================================================== */

/** USB Serial/JTAG (built-in) */
#define UART_NUM UART_NUM_0
#define UART_BAUD_RATE 115200

  /* ========================================================================
   * Grove Connector
   * ======================================================================== */

#define GROVE_SDA_PIN I2C_SDA_PIN
#define GROVE_SCL_PIN I2C_SCL_PIN

  /* ========================================================================
   * Board Features
   * ======================================================================== */

#define HAS_RGB_LED 1
#define HAS_BUTTON 1
#define HAS_BATTERY 1 /* Battery monitoring via ADC */
#define HAS_GROVE 1   /* Grove I2C connector */
#define HAS_LCD 0
#define HAS_IMU 0
#define HAS_WIFI 1
#define HAS_BLE 1

  /* ========================================================================
   * Memory Configuration
   * ======================================================================== */

#define SRAM_SIZE_KB 512
#define FLASH_SIZE_KB 4096
#define PSRAM_SIZE_KB 0 /* No PSRAM on NanoC6 */

  /* ========================================================================
   * Clock Configuration
   * ======================================================================== */

#define CPU_FREQ_MHZ 160
#define APB_FREQ_MHZ 80

/* ========================================================================
 * Power Configuration
 * ======================================================================== */

/** Battery voltage ADC channel */
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_0
#define BATTERY_ADC_ATTEN ADC_ATTEN_DB_11

/* Voltage divider: Vbat --[100K]-- ADC --[100K]-- GND */
#define BATTERY_VOLTAGE_DIVIDER 2.0f

  /* ========================================================================
   * Board Initialization
   * ======================================================================== */

  /**
   * @brief Initialize board-specific peripherals
   *
   * Call this function early in main() to set up:
   * - LED pin
   * - Button pin with pullup
   * - I2C bus
   * - Battery monitoring
   */
  void board_init(void);

  /**
   * @brief Read battery voltage
   *
   * @return Battery voltage in millivolts, or 0 if not available
   */
  uint32_t board_get_battery_voltage_mv(void);

  /**
   * @brief Check if button is pressed
   *
   * @return true if button is pressed, false otherwise
   */
  bool board_button_is_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_NANOC6_H */
