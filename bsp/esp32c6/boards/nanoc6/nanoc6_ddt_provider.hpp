/**
 * @file nanoc6_ddt_provider.hpp
 * @brief DDT Provider for M5Stack NanoC6
 *
 * Board-specific device descriptor table for M5Stack NanoC6.
 * Describes available hardware devices on this specific board.
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0
 */

#ifndef NANOC6_DDT_PROVIDER_HPP
#define NANOC6_DDT_PROVIDER_HPP

#include "v4std/ddt.hpp"
#include "v4std/ddt_types.h"

namespace v4rtos
{

/**
 * @brief DDT Provider for M5Stack NanoC6
 *
 * Board: M5Stack NanoC6
 * MCU: ESP32-C6
 *
 * Provides device descriptors for:
 * - STATUS LED (GPIO7, active-high)
 * - USER BUTTON (GPIO9, active-low)
 */
class NanoC6DdtProvider : public v4std::DdtProvider
{
 public:
  v4std::span<const v4dev_desc_t> get_devices() const override;
};

}  // namespace v4rtos

#endif  // NANOC6_DDT_PROVIDER_HPP
