# Changelog

All notable changes to V4 Runtime will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.5.2] - 2026-09-10

### Changed
- Upgrade CI checkout to v5, upload-artifact to v6 and paths-filter to v4,
  all using Node.js 24. ESP-IDF and firmware implementation are unchanged.
- Update both CI dependency pins to HAL 0.2.2, which also updates checkout.

## [0.5.1] - 2026-09-10

### Fixed
- Give isolated size-build containers an explicit disposable `XDG_CACHE_HOME`
  under `/tmp`. GitHub runner UID 1001 is absent from the SDK image's passwd
  database; its default cache path resolved to unwritable `/.cache`, causing
  baseline configuration to fail. Keep the invoking UID/GID, read-only dependency
  mounts and network isolation; no root execution or host cache is needed.

### Validation
- 24 size reporter tests and 8 build-path tests pass, along with formatting checks.
  IDF 5.5.5 configuration succeeds under unregistered UID/GID 1001 with the fixed
  cache path and networking disabled. Full firmware validation runs in CI.

## [0.5.0] - 2026-09-10

### Added
- `diagnostic-logs` measurement restores dynamic/tag logging and INFO traffic traces explicitly; CI can report changed configurations without calculating an invalid same-configuration delta.
- Explicit `quiet-logs` measurement combines static log levels with DEBUG-only transport tracing, matching the adopted defaults while leaving wireless coexistence and standard panic diagnostics enabled. Manual size CI also retains this combined profile.
- Size profiles for static logging, DEBUG-only transport tracing and disabled wireless coexistence; effective sdkconfig/compiler checks and optional manual CI artifacts. Logging default adoption is described below.
- `V4_LINK_VERBOSE_LOGS=OFF` moves only per-transfer INFO/hex tracing to DEBUG; startup and error logs are retained.
- Opt-in `V4_PANIC_DIAGNOSTICS=OFF` for ESP32-C6's direct-source engine build. Standard engine printing is omitted; runtime's ESP_LOG/LED callback, snapshots and link error handling remain unchanged. Default is ON.
- Size builds accept `--panic-diagnostics on|off`, verify the effective engine preprocessor macro, and retain an additional OFF artifact in CI. Cross-configuration comparisons remain rejected.
- Isolated ESP32-C6 Docker size builds from tracked working-tree snapshots, without hardware, developer configs or container network access.
- JSON firmware/IDF memory reports, retained ELF/map/configuration/command evidence, strict same-configuration comparison and optional application-image growth budgets.
- Firmware Size CI base/current comparisons using the current harness and pinned dependencies, reporter tests and 30-day artifacts; `make size-build` entry point.

### Compatibility
- Adopt quiet-logs for new builds: static log levels, no per-tag runtime level changes, and DEBUG-only traffic tracing. Startup INFO, ERROR, task support, standard panic output, SDK, optimization and wireless coexistence settings are retained. Existing sdkconfig/CMake caches require explicit migration; diagnostic settings remain available. Hardware validation remains pending.

### Fixed
- Pin firmware and size CI to V4-hal 0.2.1 (`1249623`), including the ESP-IDF
  UART configuration initializer warning fix.
- Correct native sibling dependency paths and share deterministic dependency
  resolution across engine/HAL/link, with explicit CMake/environment overrides
  and actionable errors for invalid paths. CI `_deps` takes precedence over
  automatic Docker/local fallbacks.
- Anchor ESP32-C6 Make build paths and the Compose file to the repository instead
  of the caller's directory. Ordinary Compose builds no longer require a serial
  device, SSH keys, unused V4-std mount or an SDK-tool cache volume. Flash/monitor
  device access is opt-in through `docker-compose.device.yml`; the IDF working
  directory is fixed (legacy `PROJECT_DIR` overrides are no longer used).
- Preserve arguments of `-u`, `-Xlinker`, entry-point and related linker options in size comparison identity, including response files. Missing required option arguments now fail instead of disappearing from the comparison.

### Validation
- Build-path refactoring: 8 host-only path tests (also added to CI), 23 size-tool
  tests and formatting checks pass. A clean IDF 5.5.5 build and a subsequent
  device-free Compose build from `/tmp` pass without using developer `.env` files.
  Application BIN remains 137,568 B, data 3,932 B and BSS 20,072 B with the local
  HAL UART fix. Hardware flashing and new remote CI runs remain untested.
- Adoption validation: clean native-default measurement is 137,568 B with 3,932 B data and 20,072 B BSS; diagnostic-logs restores the old 139,408 B / 3,956 B / 20,352 B values. The old 0.4.0 default also builds with the current harness. Strict comparison rejects this configuration transition, while CI reports separate absolute sizes without a delta. Reporter tests pass (23); hardware remains untested.
- Before default adoption, same-source default/quiet-logs clean builds passed: combined logging changes reduced application BIN by 1,840 B (139,408 to 137,568), static data+BSS by 304 B and DIRAM use by 820 B. Bootloader was unchanged; coexistence, standard panic output and runtime callback/startup/error diagnostics remained linked. That experiment passed 20 reporter tests and formatting checks without hardware testing.
- Four same-source clean IDF 5.5.5 builds pass: default app 139,408 B; static logs 138,176 B (-1,232), quiet transport 138,800 B (-608), no coexistence 136,960 B (-2,448). Static data+BSS savings are 280/16/392 B respectively; all bootloaders remain 20,576 B. These are separate, non-additive, opt-in feature tradeoffs, not new defaults.
- 19 reporter tests, formatting checks and 3 existing host link tests pass. All three cross-profile comparisons correctly reject incompatible configurations. Hardware and combined-profile validation remain pending.
- Panic ON/OFF clean builds under IDF 5.5.5 pass; application BIN shrinks by 448 B (139,408 to 138,960), while bootloader, DIRAM data/BSS/use remain unchanged. The map-based image estimate decreases by 680 B; this is distinct from BIN size. Runtime callback symbols/log strings remain in the OFF ELF.
- Engine host tests pass in both modes (14 each); link tests pass with diagnostics OFF (3), including VM_ERROR after a returning panic callback. Reporter tests pass (14). Hardware panic recovery has not been tested.
- The current harness also builds the old 0.4.0 runtime with inherited ON diagnostics; strict baseline/current ON comparison passes with zero size growth.
- Two clean IDF 5.5.5 builds (0.4.0 baseline and 0.5.0 working tree) pass with the same harness and dependencies; strict comparison passes with a zero-byte growth budget. Application 139,408 B, bootloader 20,576 B and IDF DIRAM use 65,394 B are unchanged.
- 12 reporter tests and formatting checks pass locally. The new GitHub workflow has not yet been run remotely.

## [0.4.0] - 2026-09-10

### Changed
- Standardize the ESP32-C6 SDK on ESP-IDF 5.5.5 across CI, Docker, component requirements and installation instructions.
- Pin CI's engine (0.18.1, including the ESP32 panic format fix), HAL and link checkouts to tested commits instead of tracking moving branch heads; omit the unused V4-std checkout.
- Preserve ELF, linker map, generated sdkconfig and project metadata alongside firmware artifacts.
- Correct the sdkconfig.defaults comment: silent assertions do not enable LTO.

### Validation
- Clean builds pass with IDF 5.3.0 and 5.5.5 using identical dependency source; the final 0.4.0/5.5.5 build also passes. The application image grows from 135,328 to 139,408 bytes. See [the validation report](bsp/esp32c6/IDF-VALIDATION.md) for remaining warnings and configuration caveats.
- No hardware testing was performed; boot, USB communication, scheduling and panic/LED behavior remain unverified on a device.

## [0.3.2] - 2026-09-10

### Fixed
- Use engine errors.def messages for panic diagnostics instead of an inconsistent local numeric mapping.
- Return from the VM panic callback after logging and lighting the LED, allowing V4-link 0.5 to send the execution error and accept inspection/RESET commands. VM state is not rolled back.
- Align CMake and firmware version reporting with the 0.3.x release series; remove the stale 1.0.0/1.0.0-dev placeholders.

### Compatibility
- Detailed host error reporting requires V4-link 0.5.0 and v4_cli 0.6.0. Firmware must be rebuilt and flashed to use the new behavior.

## [0.3.1] - 2025-11-05

### Added
- **V4 panic handler integration** for ESP32-C6 runtime
  - Comprehensive panic information logging via ESP_LOGE
  - Displays error code, task ID, instruction pointer, stack state
  - Shows top 4 data stack values for debugging
  - Visual error indication via rapid LED blinking
  - System halt on fatal errors for safe debugging

### Changed
- **Updated to V4-engine v0.13.0** for improved panic handler API
  - Use `V4PanicInfo` struct with correct field names
  - Use `PRId32` format specifier for portable int32_t formatting
- Fixed Docker compose V4-engine mount path (V4 → V4-engine)

### Fixed
- Fixed compilation errors in panic handler implementation
  - Added missing LED control functions
  - Added `<cinttypes>` include for PRId32
  - Corrected PanicInfo field access

## [0.2.0] - 2025-11-05

### Changed
- **BREAKING**: Renamed project from V4-rtos to V4-runtime
- **BREAKING**: Changed project description from "Real-Time Operating System" to "Runtime Environment"
- Clarified that V4 Runtime uses FreeRTOS as backend scheduler (not a standalone RTOS)
- Updated all documentation to reflect FreeRTOS-based architecture
- Comparison table now compares with Forth environments (Mecrisp, Zeptoforth, FlashForth) instead of RTOSes
- Updated architecture documentation to show actual bsp/esp32c6/runtime structure

### Removed
- **BREAKING**: Removed kernel/ directory (V4 VM used directly, no wrapper needed)
- **BREAKING**: Removed compiler/ directory (empty, no implementation)
- **BREAKING**: Removed shell/ directory (empty, no implementation)
- **BREAKING**: Removed protocol/ directory (empty, actual V4-link implementation in bsp/esp32c6/runtime)
- **BREAKING**: Removed examples/ directory (empty, actual examples in tools/examples)
- Simplified CMakeLists.txt by removing build options for removed components

### Fixed
- Updated all GitHub repository URLs from V4-project/V4-rtos to V4-project/V4-runtime
- Updated CHANGELOGs in all subdirectories with new repository URLs

### Technical Details
- Deleted ~9,070 lines of unused/duplicate code
- Project structure now accurately reflects implementation reality
- Actual runtime lives in bsp/esp32c6/runtime/ using V4 VM + FreeRTOS directly

## [0.1.0] - 2025-11-04

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

[Unreleased]: https://github.com/V4-project/V4-runtime/compare/v0.5.2...HEAD
[0.5.2]: https://github.com/V4-project/V4-runtime/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/V4-project/V4-runtime/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/V4-project/V4-runtime/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/V4-project/V4-runtime/compare/v0.3.1...v0.4.0
[0.3.1]: https://github.com/V4-project/V4-runtime/compare/v0.2.0...v0.3.1
[0.2.0]: https://github.com/V4-project/V4-runtime/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/V4-project/V4-runtime/compare/v0.0.0...v0.1.0
[0.0.0]: https://github.com/V4-project/V4-runtime/releases/tag/v0.0.0
