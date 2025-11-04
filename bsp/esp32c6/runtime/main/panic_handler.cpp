/**
 * @file panic_handler.cpp
 * @brief V4 VM panic handler implementation for ESP32-C6
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include "panic_handler.hpp"

#include <cstdio>
#include <cstring>

// Board definitions
extern "C"
{
#include "nanoc6/board.h"
}

// V4 VM API
#include "v4/vm_api.h"

// ESP-IDF APIs
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "v4-panic";

/**
 * @brief Get human-readable string for VM error code
 */
static const char* get_error_name(v4_err code)
{
  switch (code)
  {
    case 0:
      return "OK";
    case -1:
      return "NOT_FOUND";
    case -2:
      return "INVALID_OP";
    case -3:
      return "STACK_OVERFLOW";
    case -4:
      return "STACK_UNDERFLOW";
    case -5:
      return "DIV_BY_ZERO";
    case -6:
      return "OUT_OF_MEMORY";
    case -16:
      return "INVALID_ARG";
    case -32:
      return "TASK_LIMIT";
    case -33:
      return "TASK_INVALID_ID";
    case -48:
      return "MSG_QUEUE_FULL";
    case -49:
      return "MSG_NO_DATA";
    default:
      return "UNKNOWN";
  }
}

/**
 * @brief Panic handler callback
 *
 * Called by VM when a fatal error occurs.
 * Logs error details and provides visual indication via LED.
 */
static void handle_panic(void* user_data, const struct PanicInfo* info)
{
  (void)user_data;  // Unused

  if (!info)
  {
    ESP_LOGE(TAG, "!!! VM PANIC (NULL panic info) !!!");
    return;
  }

  // Log panic header
  ESP_LOGE(TAG, "");
  ESP_LOGE(TAG, "╔═══════════════════════════════════════════════════════════╗");
  ESP_LOGE(TAG, "║              V4 VM PANIC - FATAL ERROR                    ║");
  ESP_LOGE(TAG, "╚═══════════════════════════════════════════════════════════╝");

  // Log error code and name
  ESP_LOGE(TAG, "Error Code:    %d (%s)", info->error_code,
           get_error_name(info->error_code));

  // Log task information
  ESP_LOGE(TAG, "Task ID:       %d", info->task_id);

  // Log instruction pointer
  if (info->ip != 0xFFFFFFFF)
  {
    ESP_LOGE(TAG, "IP (Instr Ptr): 0x%08X", (unsigned int)info->ip);
  }

  // Log error message if available
  if (info->message && info->message[0] != '\0')
  {
    ESP_LOGE(TAG, "Message:       %s", info->message);
  }

  // Log stack state
  ESP_LOGE(TAG, "Stack Depth:   %d / 256", info->stack_depth);
  ESP_LOGE(TAG, "Return Depth:  %d / 64", info->return_depth);

  // Log top stack values (up to 4)
  if (info->stack_depth > 0)
  {
    ESP_LOGE(TAG, "Stack Values:");
    int count = info->stack_depth < 4 ? info->stack_depth : 4;
    for (int i = 0; i < count; i++)
    {
      ESP_LOGE(TAG, "  [%d]: 0x%08X (%d)", i, (unsigned int)info->stack[i],
               (int)info->stack[i]);
    }
    if (info->stack_depth > 4)
    {
      ESP_LOGE(TAG, "  ... (%d more values)", info->stack_depth - 4);
    }
  }

  ESP_LOGE(TAG, "");
  ESP_LOGE(TAG, "System halted. Reset required.");
  ESP_LOGE(TAG, "");

  // Visual indication: rapid LED blinking
  // Infinite loop to halt execution
  while (1)
  {
    board_led_on();
    vTaskDelay(pdMS_TO_TICKS(100));
    board_led_off();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

extern "C" void panic_handler_init(struct Vm* vm)
{
  if (!vm)
  {
    ESP_LOGE(TAG, "Cannot initialize panic handler: NULL VM");
    return;
  }

  vm_set_panic_handler(vm, handle_panic, nullptr);
  ESP_LOGI(TAG, "Panic handler registered");
}
