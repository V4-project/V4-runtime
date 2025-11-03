// V4-link port for ESP32-C6 USB Serial/JTAG
//
// Provides bytecode transfer over USB Serial/JTAG interface
//
// SPDX-License-Identifier: MIT OR Apache-2.0

#pragma once

#include <cstddef>
#include <memory>

// Forward declarations
extern "C"
{
  typedef struct Vm Vm;
}

namespace v4
{
namespace link
{
class Link;
}
}  // namespace v4

namespace v4rtos
{

/**
 * @brief V4-link port for ESP32-C6 USB Serial/JTAG
 *
 * Wraps V4-link protocol implementation and handles USB Serial/JTAG I/O.
 * Non-blocking design suitable for polling from main loop.
 */
class Esp32c6LinkPort
{
 public:
  /**
   * @brief Construct V4-link port
   * @param vm V4 VM instance
   * @param buffer_size V4-link receive buffer size (default: 512 bytes)
   */
  Esp32c6LinkPort(Vm* vm, size_t buffer_size = 512);

  /**
   * @brief Destructor
   */
  ~Esp32c6LinkPort();

  /**
   * @brief Poll for incoming data (non-blocking)
   *
   * Reads from USB Serial/JTAG and feeds bytes to V4-link.
   * Should be called regularly from main loop.
   */
  void poll();

  /**
   * @brief Reset V4-link state
   */
  void reset();

  /**
   * @brief Get V4-link buffer capacity
   * @return Buffer capacity in bytes
   */
  size_t buffer_capacity() const;

 private:
  std::unique_ptr<v4::link::Link> link_;        ///< V4-link instance
  static constexpr size_t USB_BUF_SIZE = 1024;  ///< USB driver buffer size
};

}  // namespace v4rtos
