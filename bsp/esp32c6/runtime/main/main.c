/**
 * @file main.c
 * @brief V4 RTOS Runtime for ESP32-C6
 *
 * This runtime provides:
 * - V4 VM initialization
 * - Preemptive scheduler
 * - Bytecode reception via UART (V4-link protocol)
 * - Bytecode execution
 *
 * Flash this once to the device, then send bytecode from host using v4_cli.
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "../boards/nanoc6/board.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "v4-runtime";

// Placeholder for V4 VM structures (will be replaced with actual kernel implementation)
typedef struct
{
  uint8_t bytecode[4096];  // Bytecode buffer
  size_t bytecode_len;
  uint32_t pc;  // Program counter
  uint32_t sp;  // Stack pointer
} v4_vm_t;

static v4_vm_t vm;

/**
 * @brief Initialize V4 VM
 */
static void v4_vm_init(v4_vm_t *vm)
{
  memset(vm, 0, sizeof(v4_vm_t));
  ESP_LOGI(TAG, "V4 VM initialized");
}

/**
 * @brief Receive bytecode via UART
 *
 * This is a placeholder for V4-link protocol.
 * In the full implementation, this would use the protocol component.
 */
static int receive_bytecode(v4_vm_t *vm)
{
  ESP_LOGI(TAG, "Waiting for bytecode...");

  // For now, just wait for any UART input
  uint8_t buffer[128];
  int len = uart_read_bytes(UART_NUM_0, buffer, sizeof(buffer), pdMS_TO_TICKS(5000));

  if (len > 0)
  {
    ESP_LOGI(TAG, "Received %d bytes", len);
    // In real implementation, parse V4-link frames and extract bytecode
    memcpy(vm->bytecode, buffer, len < sizeof(vm->bytecode) ? len : sizeof(vm->bytecode));
    vm->bytecode_len = len;
    return 0;
  }

  return -1;  // Timeout
}

/**
 * @brief Execute bytecode in VM
 *
 * Placeholder for actual VM execution.
 * Will be replaced with kernel/src/vm.c implementation.
 */
static void v4_vm_execute(v4_vm_t *vm)
{
  ESP_LOGI(TAG, "Executing bytecode (%zu bytes)", vm->bytecode_len);

  // Placeholder: just echo received data
  printf("Bytecode: ");
  for (size_t i = 0; i < vm->bytecode_len && i < 16; i++)
  {
    printf("%02x ", vm->bytecode[i]);
  }
  printf("\n");

  // TODO: Actual VM execution loop will go here
  // while (vm->pc < vm->bytecode_len) {
  //     execute_instruction(&vm);
  // }

  ESP_LOGI(TAG, "Execution complete");
}

/**
 * @brief Initialize board GPIO
 */
static void board_init(void)
{
  // Configure LED for status indication
  gpio_config_t led_conf = {
      .pin_bit_mask = (1ULL << LED_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&led_conf);

  // LED on to indicate runtime is ready
  gpio_set_level(LED_PIN, 1);

  ESP_LOGI(TAG, "Board initialized");
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
  ESP_LOGI(TAG, "=== V4 RTOS Runtime ===");
  ESP_LOGI(TAG, "Board: %s", BOARD_NAME);
  ESP_LOGI(TAG, "Version: 1.0.0-dev");

  // Initialize board
  board_init();

  // Initialize V4 VM
  v4_vm_init(&vm);

  ESP_LOGI(TAG, "Runtime ready. Send bytecode via UART.");
  ESP_LOGI(TAG, "Use: v4flash -p /dev/ttyUSB0 program.bin");

  // Main loop: receive and execute bytecode
  while (1)
  {
    if (receive_bytecode(&vm) == 0)
    {
      v4_vm_execute(&vm);
    }
  }
}
