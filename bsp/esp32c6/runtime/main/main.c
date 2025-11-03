/**
 * @file main.c
 * @brief V4 RTOS Runtime for ESP32-C6
 *
 * This runtime provides:
 * - V4 VM initialization with kernel APIs
 * - Preemptive task scheduler (10ms time slice)
 * - HAL initialization for peripherals
 * - Bytecode reception via UART (V4-link protocol - future)
 * - Bytecode execution
 *
 * Flash this once to the device, then send bytecode from host using v4_cli.
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include <stdio.h>
#include <string.h>

// Board definitions
#include "nanoc6/board.h"
#include "nanoc6/peripherals.h"

// V4 kernel APIs
#include "v4/task.h"
#include "v4/vm_api.h"

// V4-hal APIs
#include "v4/hal.h"

// ESP-IDF APIs
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "v4-runtime";

// ==============================================================================
// VM Memory Configuration
// ==============================================================================

/**
 * @brief V4 VM arena size
 *
 * This is the memory available for:
 * - Dictionary (compiled words)
 * - Data stack
 * - Return stack
 * - Temporary allocations
 *
 * 16KB is sufficient for basic RTOS operations.
 * Increase if running complex Forth programs.
 */
#define VM_ARENA_SIZE (16 * 1024)

/** VM memory arena (statically allocated) */
static uint8_t vm_arena[VM_ARENA_SIZE] __attribute__((aligned(4)));

/** Global VM instance */
static struct Vm *g_vm = NULL;

// ==============================================================================
// V4 VM Initialization
// ==============================================================================

/**
 * @brief Initialize V4 VM and task system
 *
 * Creates a VM instance with the configured arena and initializes
 * the preemptive task scheduler with a 10ms time slice.
 *
 * @return 0 on success, negative error code on failure
 */
static int v4_init(void)
{
  // Configure VM with static arena
  VmConfig config = {
      .mem = vm_arena,
      .mem_size = VM_ARENA_SIZE,
      .mmio = NULL,  // No MMIO windows for now
      .mmio_count = 0,
      .arena = NULL  // Use malloc for word names (ESP-IDF heap)
  };

  // Create VM instance
  g_vm = vm_create(&config);
  if (g_vm == NULL)
  {
    ESP_LOGE(TAG, "Failed to create VM instance");
    return -1;
  }

  ESP_LOGI(TAG, "V4 VM created (arena: %d KB)", VM_ARENA_SIZE / 1024);

  // Initialize task system with 10ms time slice
  v4_err err = vm_task_init(g_vm, 10);
  if (err != 0)
  {
    ESP_LOGE(TAG, "Failed to initialize task system: %d", err);
    return -2;
  }

  ESP_LOGI(TAG, "V4 task scheduler initialized (10ms time slice)");

  return 0;
}

// ==============================================================================
// V4-link Protocol (Future Implementation)
// ==============================================================================

/**
 * @brief Receive bytecode via V4-link protocol
 *
 * This is a placeholder for V4-link protocol implementation.
 * In the future, this will:
 * 1. Listen for V4-link frames on UART
 * 2. Parse frame headers and extract bytecode
 * 3. Load bytecode into VM dictionary
 * 4. Acknowledge successful reception
 *
 * @return 0 on success, negative on error or timeout
 */
static int v4_link_receive(void)
{
  ESP_LOGI(TAG, "Waiting for bytecode (V4-link protocol)...");

  // TODO: Implement V4-link protocol
  // For now, just wait for UART input as a placeholder
  uint8_t buffer[128];
  int len = uart_read_bytes(UART_NUM_0, buffer, sizeof(buffer), pdMS_TO_TICKS(5000));

  if (len > 0)
  {
    ESP_LOGI(TAG, "Received %d bytes (raw UART data)", len);
    // TODO: Parse V4-link frames and load into VM
    return 0;
  }

  return -1;  // Timeout
}

// ==============================================================================
// Board Initialization
// ==============================================================================

/**
 * @brief Initialize board peripherals
 *
 * Initializes:
 * - LED (GPIO7) for status indication
 * - Button (GPIO9) with pullup
 * - RGB LED (GPIO8) for future use
 *
 * @return 0 on success, negative on error
 */
static int board_init(void)
{
  esp_err_t ret = board_peripherals_init();
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to initialize board peripherals: %d", ret);
    return -1;
  }

  // LED on to indicate runtime is starting
  board_led_on();

  ESP_LOGI(TAG, "Board: %s", BOARD_NAME);
  ESP_LOGI(TAG, "MCU: %s @ %d MHz", BOARD_MCU, CPU_FREQ_MHZ);
  ESP_LOGI(TAG, "RAM: %d KB, Flash: %d KB", SRAM_SIZE_KB, FLASH_SIZE_KB);

  return 0;
}

// ==============================================================================
// Main Entry Point
// ==============================================================================

/**
 * @brief Main application entry point
 *
 * Initialization sequence:
 * 1. HAL initialization (V4-hal)
 * 2. Board peripheral initialization
 * 3. V4 VM creation and task system initialization
 * 4. Main loop waiting for V4-link bytecode
 */
void app_main(void)
{
  ESP_LOGI(TAG, "=== V4 RTOS Runtime ===");
  ESP_LOGI(TAG, "Version: 1.0.0-dev");

  // Step 1: Initialize HAL
  int hal_status = hal_init();
  if (hal_status != 0)
  {
    ESP_LOGE(TAG, "HAL initialization failed: %d", hal_status);
    ESP_LOGE(TAG, "System halted.");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  ESP_LOGI(TAG, "HAL initialized");

  // Step 2: Initialize board peripherals
  if (board_init() != 0)
  {
    ESP_LOGE(TAG, "Board initialization failed");
    ESP_LOGE(TAG, "System halted.");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  // Step 3: Initialize V4 VM and task system
  if (v4_init() != 0)
  {
    ESP_LOGE(TAG, "V4 initialization failed");
    ESP_LOGE(TAG, "System halted.");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  // All systems ready
  ESP_LOGI(TAG, "=== V4 RTOS Runtime Ready ===");
  ESP_LOGI(TAG, "Waiting for bytecode via V4-link protocol...");
  ESP_LOGI(TAG, "Use: v4flash -p /dev/ttyACM0 program.bin");

  // LED blink pattern to indicate ready state
  board_led_off();
  vTaskDelay(pdMS_TO_TICKS(100));
  board_led_on();
  vTaskDelay(pdMS_TO_TICKS(100));
  board_led_off();

  // Main loop: wait for V4-link bytecode
  while (1)
  {
    if (v4_link_receive() == 0)
    {
      ESP_LOGI(TAG, "Bytecode received and loaded");
      // TODO: Execute bytecode via VM
    }
  }
}
