# Changelog

All notable changes to V4 RTOS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive kernel unit tests using doctest (copied from V4)
  - VM wrapper tests (create, destroy, initialization)
  - Task spawn tests (single, multiple, max tasks, error handling)
  - Task lifecycle tests (get_info, self, critical sections)
  - Message passing tests (send/receive, queue full, broadcast)

### Changed
- **BREAKING**: Converted kernel to C++ (was C) to leverage V4's C++ implementation
- **BREAKING**: Removed duplicate scheduler/task/message implementations (~900 lines)
- Kernel now uses V4's built-in scheduler, task manager, and message queue directly
- Thin C++ wrapper (vm_wrapper.cpp) replaces standalone kernel implementation
- CMake configuration simplified to use V4 VM library with FetchContent support
- CI workflows consolidated from 4 separate workflows to 1 unified workflow
- ESP32-C6 runtime: Disabled heartbeat LED to allow full bytecode control of GPIO7

### Removed
- kernel/src/scheduler.c (167 lines) - now uses V4's scheduler.cpp
- kernel/src/task.c (125 lines) - now uses V4's task.cpp
- kernel/src/message.c (115 lines) - now uses V4's message.cpp
- kernel/include/v4/task.h - duplicate type definitions removed

### Technical Details
- V4-RTOS is now a thin wrapper around V4 VM with 10ms time slice initialization
- Direct access to V4 internals (Vm struct) for comprehensive testing
- Uses V4 v0.10.0 with C-compatible errors.h and RTOS error codes
- All tests pass with doctest framework (DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS)

## [0.0.0] - 2025-01-03

### Added
- Initial repository structure
- Monorepo framework for kernel, hal, compiler, shell, protocol components
- Build system (CMake + Makefile wrapper)
- Documentation skeleton
- MIT + Apache 2.0 dual licensing
