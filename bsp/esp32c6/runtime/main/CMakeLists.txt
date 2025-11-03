# Main component CMakeLists.txt
#
# Integrates V4 kernel and V4-hal into ESP-IDF build system
#
# SPDX-License-Identifier: MIT OR Apache-2.0

# ==============================================================================
# V4 Kernel Integration
# ==============================================================================

# Try multiple possible locations for V4 and V4-hal 1. CI: runtime/_deps/V4 (for Docker
# volume mount) 2. Local: ../../../../V4 (parent directories)

set(V4_DIR_CI "${CMAKE_CURRENT_SOURCE_DIR}/../_deps/V4")
set(V4_DIR_LOCAL "${CMAKE_CURRENT_SOURCE_DIR}/../../../../V4")
set(V4HAL_DIR_CI "${CMAKE_CURRENT_SOURCE_DIR}/../_deps/V4-hal")
set(V4HAL_DIR_LOCAL "${CMAKE_CURRENT_SOURCE_DIR}/../../../../V4-hal")

# Check V4 location
if(EXISTS "${V4_DIR_CI}/include/v4/vm_api.h")
  set(V4_DIR "${V4_DIR_CI}")
  message(STATUS "Found V4 at ${V4_DIR} (CI location)")
elseif(EXISTS "${V4_DIR_LOCAL}/include/v4/vm_api.h")
  set(V4_DIR "${V4_DIR_LOCAL}")
  message(STATUS "Found V4 at ${V4_DIR} (local location)")
else()
  message(FATAL_ERROR "V4 not found. Please clone V4 repository to:\n"
                      "  - ${V4_DIR_CI} (CI build)\n" "  - ${V4_DIR_LOCAL} (local build)")
endif()

# Check V4-hal location
if(EXISTS "${V4HAL_DIR_CI}/include/v4/hal.h")
  set(V4HAL_DIR "${V4HAL_DIR_CI}")
  message(STATUS "Found V4-hal at ${V4HAL_DIR} (CI location)")
elseif(EXISTS "${V4HAL_DIR_LOCAL}/include/v4/hal.h")
  set(V4HAL_DIR "${V4HAL_DIR_LOCAL}")
  message(STATUS "Found V4-hal at ${V4HAL_DIR} (local location)")
else()
  message(
    FATAL_ERROR "V4-hal not found. Please clone V4-hal repository to:\n"
                "  - ${V4HAL_DIR_CI} (CI build)\n" "  - ${V4HAL_DIR_LOCAL} (local build)")
endif()

# V4 source files (minimal set for runtime)
set(V4_SRCS
    "${V4_DIR}/src/core.cpp"
    "${V4_DIR}/src/arena.cpp"
    "${V4_DIR}/src/hal_wrapper.cpp"
    "${V4_DIR}/src/memory.cpp"
    "${V4_DIR}/src/message.cpp"
    "${V4_DIR}/src/scheduler.cpp"
    "${V4_DIR}/src/task.cpp")

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
  "../../boards"
  "${V4_DIR}/include"
  "${V4HAL_DIR}/include"
  "${V4HAL_DIR}/ports/esp32"
  REQUIRES
  driver
  freertos
  esp_system
  esp_timer
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
