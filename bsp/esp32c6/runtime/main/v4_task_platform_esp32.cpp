// V4 task platform implementation for ESP32/FreeRTOS
//
// Provides FreeRTOS-based task system backend for V4 kernel
//
// SPDX-License-Identifier: MIT OR Apache-2.0

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Static spinlock for critical sections
static portMUX_TYPE v4_task_critical_spinlock = portMUX_INITIALIZER_UNLOCKED;

extern "C"
{
  /**
   * @brief Get current tick time in milliseconds
   *
   * Used by V4 task scheduler for time-slicing and sleep.
   *
   * @return Current time in milliseconds since boot
   */
  uint32_t v4_task_platform_get_tick_ms(void)
  {
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
  }

  /**
   * @brief Enter critical section
   *
   * Disables interrupts and acquires spinlock. Supports nesting.
   * Each call must be paired with v4_task_platform_critical_exit().
   */
  void v4_task_platform_critical_enter(void)
  {
    portENTER_CRITICAL(&v4_task_critical_spinlock);
  }

  /**
   * @brief Exit critical section
   *
   * Re-enables interrupts and releases spinlock.
   */
  void v4_task_platform_critical_exit(void)
  {
    portEXIT_CRITICAL(&v4_task_critical_spinlock);
  }

}  // extern "C"
