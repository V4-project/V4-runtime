cmake_minimum_required(VERSION 3.15)
project(
  V4-runtime
  VERSION 0.3.2
  LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(V4_BUILD_HAL "Build HAL integration" ON)
option(V4_BUILD_TESTS "Build tests" OFF)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-exceptions -fno-rtti")

# Components
if(V4_BUILD_HAL)
  add_subdirectory(hal)
endif()

# Tests
if(V4_BUILD_TESTS)
  enable_testing()
endif()
