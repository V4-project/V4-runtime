/**
 * @file panic_handler.hpp
 * @brief V4 VM panic handler for ESP32-C6 runtime
 *
 * Provides panic handler integration for V4 VM:
 * - Logs panic information via ESP_LOGE
 * - Provides visual indication via LED
 * - Formats error messages for debugging
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initialize V4 panic handler for ESP32-C6
   *
   * Registers a panic handler that:
   * - Logs error messages via ESP_LOGE
   * - Blinks LED rapidly to indicate error
   * - Formats detailed panic information
   *
   * Must be called after vm_create() and before any VM execution.
   *
   * @param vm VM instance
   */
  void panic_handler_init(struct Vm* vm);

#ifdef __cplusplus
}  // extern "C"
#endif
