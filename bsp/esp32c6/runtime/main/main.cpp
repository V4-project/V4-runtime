/**
 * @file main.cpp
 * @brief V4 RTOS Runtime for ESP32-C6
 *
 * This runtime provides:
 * - V4 VM initialization with kernel APIs
 * - Preemptive task scheduler (10ms time slice)
 * - HAL initialization for peripherals
 * - Bytecode reception via USB Serial/JTAG (V4-link protocol)
 * - Bytecode execution
 *
 * Flash this once to the device, then send bytecode from host using v4_cli.
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include <cstdio>
#include <cstring>

// Board definitions
extern "C"
{
#include "nanoc6/board.h"
#include "nanoc6/peripherals.h"
}

// V4 kernel APIs
#include "v4/task.h"
#include "v4/vm_api.h"

// V4-hal APIs
#include "v4/hal.h"

// V4-link port
#include "v4_link_port.hpp"

// V4 panic handler
#include "panic_handler.hpp"

// ESP-IDF APIs
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "v4-runtime";

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
static struct Vm* g_vm = nullptr;

/** Global V4-link port instance */
static v4rtos::Esp32c6LinkPort* g_link = nullptr;

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
      .mmio = nullptr,  // No MMIO windows for now
      .mmio_count = 0,
      .arena = nullptr  // Use malloc for word names (ESP-IDF heap)
  };

  // Create VM instance
  g_vm = vm_create(&config);
  if (g_vm == nullptr)
  {
    ESP_LOGE(TAG, "Failed to create VM instance");
    return -1;
  }

  ESP_LOGI(TAG, "V4 VM created (arena: %d KB)", VM_ARENA_SIZE / 1024);

  // Register panic handler for fatal errors
  panic_handler_init(g_vm);

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
// Board Initialization
// ==============================================================================

/**
 * @brief Initialize board peripherals
 *
 * Initializes:
 * - LED (GPIO7) for status indication
 * - Button (GPIO9) with pullup
 * - RGB LED (GPIO8) for future use
 */
static void board_init_runtime(void)
{
  esp_err_t ret = board_peripherals_init();
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to initialize board peripherals: %d", ret);
    ESP_LOGE(TAG, "System halted.");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  // LED on to indicate runtime is starting
  board_led_on();

  ESP_LOGI(TAG, "Board: %s", BOARD_NAME);
  ESP_LOGI(TAG, "MCU: %s @ %d MHz", BOARD_MCU, CPU_FREQ_MHZ);
  ESP_LOGI(TAG, "RAM: %d KB, Flash: %d KB", SRAM_SIZE_KB, FLASH_SIZE_KB);
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
 * 4. V4-link protocol initialization
 * 5. Main loop polling for V4-link bytecode
 */
extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "=== V4 RTOS Runtime ===");
  ESP_LOGI(TAG, "Version: 1.0.0-dev");

  // Step 1: Initialize HAL
  ESP_LOGI(TAG, "[1/4] Initializing HAL...");
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
  ESP_LOGI(TAG, "[2/4] Initializing board peripherals...");
  board_init_runtime();
  // LED blink to indicate board ready
  board_led_on();
  vTaskDelay(pdMS_TO_TICKS(100));
  board_led_off();
  vTaskDelay(pdMS_TO_TICKS(200));

  // Step 3: Initialize V4 VM and task system
  ESP_LOGI(TAG, "[3/4] Initializing V4 VM and task system...");
  if (v4_init() != 0)
  {
    ESP_LOGE(TAG, "V4 initialization failed");
    ESP_LOGE(TAG, "System halted.");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  // LED blinks to indicate VM ready
  for (int i = 0; i < 2; i++)
  {
    board_led_on();
    vTaskDelay(pdMS_TO_TICKS(100));
    board_led_off();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelay(pdMS_TO_TICKS(200));

  // Step 4: Initialize V4-link protocol
  ESP_LOGI(TAG, "[4/4] Initializing V4-link protocol...");
  g_link = new v4rtos::Esp32c6LinkPort(g_vm, 512);
  if (g_link == nullptr)
  {
    ESP_LOGE(TAG, "V4-link initialization failed");
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

  // LED blink pattern to indicate ready state (3 quick blinks)
  for (int i = 0; i < 3; i++)
  {
    board_led_on();
    vTaskDelay(pdMS_TO_TICKS(100));
    board_led_off();
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  ESP_LOGI(TAG, "Starting main loop (polling for V4-link bytecode)...");

  // Main loop: poll for V4-link bytecode
  // Note: Heartbeat LED disabled to allow bytecode control of GPIO7
  while (1)
  {
    g_link->poll();
    vTaskDelay(pdMS_TO_TICKS(1));  // 1ms polling interval
  }
}
