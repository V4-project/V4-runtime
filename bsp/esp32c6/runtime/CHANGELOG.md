# Changelog - ESP32-C6 Runtime

All notable changes to the V4 RTOS ESP32-C6 runtime will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Complete V4 kernel integration via `vm_create()` and `vm_task_init()` APIs
- V4-hal integration via `hal_init()` API
- Board peripheral initialization using helper functions from `peripherals.h`
- 16KB VM arena for dictionary and stacks
- Preemptive task scheduler with 10ms time slice
- V4-link protocol placeholder (UART reception)
- LED status indication during startup
- Comprehensive error handling with system halt on critical failures
- ESP-IDF component integration via `main/CMakeLists.txt`:
  - V4 kernel sources (vm.cpp, dict.cpp, compiler.cpp, task.cpp, arena.cpp)
  - V4-hal sources (C bridge + ESP32 platform)
  - C++17 compilation with -fno-exceptions -fno-rtti
  - HAL_PLATFORM_ESP32 and V4_EMBEDDED defines
- `sdkconfig.defaults` with minimal ESP32-C6 configuration
- `idf_component.yml` for ESP-IDF v5.1+ dependency management

### Changed
- Replaced placeholder VM structures with actual V4 kernel APIs
- Updated main.c to use real V4 API calls instead of dummy implementations
- Board initialization now uses `board_peripherals_init()` helper
- Improved logging with detailed system information (MCU, RAM, Flash)

### Fixed
- N/A

### Removed
- Placeholder `v4_vm_t` structure and dummy functions

## [0.1.0] - 2025-01-03

### Added
- Initial runtime project structure
- Placeholder VM implementation
- Basic board initialization (LED)
- UART bytecode reception skeleton
- ESP-IDF project configuration (CMakeLists.txt)
- README with build instructions

[Unreleased]: https://github.com/V4-project/V4-rtos/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/V4-project/V4-rtos/releases/tag/v0.1.0
