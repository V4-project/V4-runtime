// V4-link port implementation for ESP32-C6
//
// SPDX-License-Identifier: MIT OR Apache-2.0

#include "v4_link_port.hpp"

#include <cstring>

#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "v4/vm_api.h"
#include "v4link/link.hpp"

static const char* TAG = "V4Link";

namespace v4rtos
{

// Static UART write callback for V4-link
static void usb_serial_jtag_write_callback(void* user, const uint8_t* data, size_t len)
{
  (void)user;  // Unused

  // Send response data over USB Serial/JTAG
  int written = usb_serial_jtag_write_bytes((const char*)data, len, portMAX_DELAY);
  if (written < 0)
  {
    ESP_LOGE(TAG, "Failed to write to USB Serial/JTAG");
  }
}

Esp32c6LinkPort::Esp32c6LinkPort(Vm* vm, size_t buffer_size) : link_(nullptr)
{
  ESP_LOGI(TAG, "Initializing V4-link (buffer: %d bytes)", buffer_size);

  // Configure USB Serial/JTAG driver
  usb_serial_jtag_driver_config_t usb_config = {
      .tx_buffer_size = USB_BUF_SIZE,
      .rx_buffer_size = USB_BUF_SIZE,
  };

  // Install USB Serial/JTAG driver
  esp_err_t ret = usb_serial_jtag_driver_install(&usb_config);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to install USB Serial/JTAG driver: %d", ret);
    return;
  }

  ESP_LOGI(TAG, "USB Serial/JTAG driver installed");

  // Create V4-link instance with write callback
  link_ = std::make_unique<v4::link::Link>(vm, usb_serial_jtag_write_callback, nullptr,
                                           buffer_size);

  ESP_LOGI(TAG, "V4-link initialized");
}

Esp32c6LinkPort::~Esp32c6LinkPort()
{
  // Uninstall USB Serial/JTAG driver
  usb_serial_jtag_driver_uninstall();
}

void Esp32c6LinkPort::poll()
{
  if (!link_)
  {
    return;
  }

  // Read available data from USB Serial/JTAG (non-blocking)
  uint8_t buffer[128];
  int len = usb_serial_jtag_read_bytes(buffer, sizeof(buffer), 0);

  if (len > 0)
  {
    // Feed received bytes to V4-link
    for (int i = 0; i < len; ++i)
    {
      link_->feed_byte(buffer[i]);
    }
  }
}

void Esp32c6LinkPort::reset()
{
  if (link_)
  {
    link_->reset();
    ESP_LOGI(TAG, "V4-link reset");
  }
}

size_t Esp32c6LinkPort::buffer_capacity() const
{
  return link_ ? link_->buffer_capacity() : 0;
}

}  // namespace v4rtos
