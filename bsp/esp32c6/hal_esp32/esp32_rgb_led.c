/**
 * @file esp32_rgb_led.c
 * @brief WS2812 RGB LED driver for ESP32 using RMT
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#include "esp32_rgb_led.h"

#include <string.h>

#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "led_strip_encoder.h"

static const char* TAG = "rgb_led";

static rmt_channel_handle_t g_led_chan = NULL;
static rmt_encoder_handle_t g_led_encoder = NULL;
static uint16_t g_led_count = 0;
static uint8_t* g_led_buffer = NULL;  // RGB buffer (3 bytes per LED)

esp_err_t esp32_rgb_led_init(uint8_t gpio, uint16_t led_count)
{
  esp_err_t ret;

  if (g_led_chan != NULL)
  {
    ESP_LOGW(TAG, "RGB LED already initialized");
    return ESP_ERR_INVALID_STATE;
  }

  g_led_count = led_count;

  // Allocate RGB buffer (3 bytes per LED: R, G, B)
  g_led_buffer = (uint8_t*)calloc(led_count * 3, sizeof(uint8_t));
  if (g_led_buffer == NULL)
  {
    ESP_LOGE(TAG, "Failed to allocate LED buffer");
    return ESP_ERR_NO_MEM;
  }

  // Configure RMT TX channel
  rmt_tx_channel_config_t tx_chan_config = {
      .gpio_num = gpio,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000,  // 10 MHz
      .mem_block_symbols = 64,
      .trans_queue_depth = 4,
  };

  ret = rmt_new_tx_channel(&tx_chan_config, &g_led_chan);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to create RMT TX channel: %d", ret);
    free(g_led_buffer);
    g_led_buffer = NULL;
    return ret;
  }

  // Create LED strip encoder
  led_strip_encoder_config_t encoder_config = {
      .resolution = tx_chan_config.resolution_hz,
  };

  ret = rmt_new_led_strip_encoder(&encoder_config, &g_led_encoder);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to create LED strip encoder: %d", ret);
    rmt_del_channel(g_led_chan);
    g_led_chan = NULL;
    free(g_led_buffer);
    g_led_buffer = NULL;
    return ret;
  }

  // Enable RMT TX channel
  ret = rmt_enable(g_led_chan);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to enable RMT channel: %d", ret);
    rmt_del_encoder(g_led_encoder);
    g_led_encoder = NULL;
    rmt_del_channel(g_led_chan);
    g_led_chan = NULL;
    free(g_led_buffer);
    g_led_buffer = NULL;
    return ret;
  }

  ESP_LOGI(TAG, "RGB LED initialized: GPIO %d, %d LEDs", gpio, led_count);

  // Clear all LEDs initially
  return esp32_rgb_led_clear_all();
}

esp_err_t esp32_rgb_led_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
  if (g_led_buffer == NULL)
  {
    ESP_LOGE(TAG, "RGB LED not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  if (index >= g_led_count)
  {
    ESP_LOGE(TAG, "LED index %d out of range (max %d)", index, g_led_count - 1);
    return ESP_ERR_INVALID_ARG;
  }

  // Store in buffer (GRB order for WS2812)
  uint16_t offset = index * 3;
  g_led_buffer[offset + 0] = g;  // Green
  g_led_buffer[offset + 1] = r;  // Red
  g_led_buffer[offset + 2] = b;  // Blue

  ESP_LOGD(TAG, "Set LED %d to RGB(%d, %d, %d)", index, r, g, b);

  return ESP_OK;
}

esp_err_t esp32_rgb_led_clear(uint16_t index)
{
  return esp32_rgb_led_set(index, 0, 0, 0);
}

esp_err_t esp32_rgb_led_clear_all(void)
{
  if (g_led_buffer == NULL)
  {
    ESP_LOGE(TAG, "RGB LED not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  memset(g_led_buffer, 0, g_led_count * 3);

  return esp32_rgb_led_refresh();
}

esp_err_t esp32_rgb_led_refresh(void)
{
  if (g_led_chan == NULL || g_led_encoder == NULL || g_led_buffer == NULL)
  {
    ESP_LOGE(TAG, "RGB LED not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  // Transmit RGB data
  rmt_transmit_config_t tx_config = {
      .loop_count = 0,  // No loop
  };

  esp_err_t ret =
      rmt_transmit(g_led_chan, g_led_encoder, g_led_buffer, g_led_count * 3, &tx_config);

  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to transmit LED data: %d", ret);
    return ret;
  }

  // Wait for transmission to complete
  ret = rmt_tx_wait_all_done(g_led_chan, 100);
  if (ret != ESP_OK)
  {
    ESP_LOGW(TAG, "LED transmission timeout");
  }

  return ret;
}

void esp32_rgb_led_deinit(void)
{
  if (g_led_chan != NULL)
  {
    rmt_disable(g_led_chan);
    rmt_del_channel(g_led_chan);
    g_led_chan = NULL;
  }

  if (g_led_encoder != NULL)
  {
    rmt_del_encoder(g_led_encoder);
    g_led_encoder = NULL;
  }

  if (g_led_buffer != NULL)
  {
    free(g_led_buffer);
    g_led_buffer = NULL;
  }

  g_led_count = 0;

  ESP_LOGI(TAG, "RGB LED deinitialized");
}
