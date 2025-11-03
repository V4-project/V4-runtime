# Main component CMakeLists.txt
#
# Integrates V4 kernel and V4-hal into ESP-IDF build system
#
# SPDX-License-Identifier: MIT OR Apache-2.0

# ==============================================================================
# V4 Kernel Integration
# ==============================================================================

set(V4_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../V4")
set(V4HAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../V4-hal")

# Check if V4 exists
if(NOT EXISTS "${V4_DIR}/include/v4/vm_api.h")
  message(FATAL_ERROR "V4 not found at ${V4_DIR}. Please clone V4 repository.")
endif()

# Check if V4-hal exists
if(NOT EXISTS "${V4HAL_DIR}/include/v4/hal.h")
  message(FATAL_ERROR "V4-hal not found at ${V4HAL_DIR}. Please clone V4-hal repository.")
endif()

# V4 source files (minimal set for runtime)
set(V4_SRCS "${V4_DIR}/src/vm.cpp" "${V4_DIR}/src/dict.cpp" "${V4_DIR}/src/compiler.cpp"
            "${V4_DIR}/src/task.cpp" "${V4_DIR}/src/arena.cpp")

# V4-hal source files (C bridge + ESP32 platform)
set(V4HAL_SRCS
    "${V4HAL_DIR}/src/common/hal_capabilities.cpp"
    "${V4HAL_DIR}/src/common/hal_core.cpp"
    "${V4HAL_DIR}/src/common/hal_error.cpp"
    "${V4HAL_DIR}/src/bridge/hal_gpio_bridge.cpp"
    "${V4HAL_DIR}/src/bridge/hal_uart_bridge.cpp"
    "${V4HAL_DIR}/src/bridge/hal_timer_bridge.cpp"
    "${V4HAL_DIR}/src/bridge/hal_console_bridge.cpp"
    "${V4HAL_DIR}/ports/esp32/platform_esp32.cpp")

# ==============================================================================
# Component Registration
# ==============================================================================

idf_component_register(
  SRCS
  "main.c"
  ${V4_SRCS}
  ${V4HAL_SRCS}
  INCLUDE_DIRS
  "."
  "../boards"
  "${V4_DIR}/include"
  "${V4HAL_DIR}/include"
  "${V4HAL_DIR}/ports/esp32"
  REQUIRES
  driver
  freertos
  esp_system
  log)

# ==============================================================================
# Compiler Options
# ==============================================================================

# C++17 required for V4 and V4-hal
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)

# Compiler flags for V4 and V4-hal
target_compile_options(${COMPONENT_LIB} PRIVATE -fno-exceptions -fno-rtti -Wall -Wextra
                                                $<$<COMPILE_LANGUAGE:CXX>:-Wno-register>)

# Platform defines
target_compile_definitions(${COMPONENT_LIB} PRIVATE HAL_PLATFORM_ESP32 V4_EMBEDDED)
