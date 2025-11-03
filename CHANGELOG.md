# Changelog

All notable changes to V4 RTOS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial kernel implementation with V4 VM core and preemptive multitasking
  - VM core: lifecycle management, word registration, stack operations
  - VM executor: bytecode execution with basic opcodes (stack, arithmetic, comparison, control flow)
  - Preemptive scheduler: priority-based with round-robin for equal priorities
  - Task management: spawn, sleep, exit, yield operations
  - Message queue: 16-message inter-task communication queue
  - Platform stubs for host testing (Linux/macOS)
- Code formatting configuration (.clang-format, .cmake-format.yaml, .editorconfig)
- Comprehensive Makefile with format, build, test, and sanitizer targets
- ESP32-C6 BSP structure with M5Stack NanoC6 board support
- Runtime-based deployment model (flash once, send bytecode)
- Documentation: README, architecture guide, getting started, API reference

### Changed
- Compiler and shell marked as optional components (kernel+hal are core)
- Restructured BSP for runtime-based deployment instead of example-based

### Technical Details
- 8 concurrent tasks with independent stacks (256 DS, 64 RS per task)
- Context switch: <100μs target
- Memory footprint: ~48KB minimum (kernel+hal), ~140KB full config
- Dependencies: V4 opcodes and sys_ids via CMake FetchContent

## [0.0.0] - 2025-01-03

### Added
- Initial repository structure
- Monorepo framework for kernel, hal, compiler, shell, protocol components
- Build system (CMake + Makefile wrapper)
- Documentation skeleton
- MIT + Apache 2.0 dual licensing
