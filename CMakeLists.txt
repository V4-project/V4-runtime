cmake_minimum_required(VERSION 3.15)
project(
  V4-rtos
  VERSION 1.0.0
  LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(V4_BUILD_KERNEL "Build kernel (required)" ON)
option(V4_BUILD_HAL "Build HAL (required)" ON)
option(V4_BUILD_COMPILER "Build compiler (optional)" OFF)
option(V4_BUILD_SHELL "Build shell (optional)" OFF)
option(V4_BUILD_PROTOCOL "Build protocol (optional)" OFF)
option(V4_BUILD_TESTS "Build tests" OFF)
option(V4_BUILD_EXAMPLES "Build examples" OFF)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-exceptions -fno-rtti")

# Components
if(V4_BUILD_KERNEL)
  add_subdirectory(kernel)
endif()

# HAL will be implemented later if(V4_BUILD_HAL) add_subdirectory(hal) endif()

if(V4_BUILD_COMPILER)
  add_subdirectory(compiler)
endif()

if(V4_BUILD_SHELL)
  add_subdirectory(shell)
endif()

if(V4_BUILD_PROTOCOL)
  add_subdirectory(protocol)
endif()

# Examples
if(V4_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Tests - will be implemented later if(V4_BUILD_TESTS) enable_testing() endif()
