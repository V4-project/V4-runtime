# V4-runtime Maintenance Notes

Updated 2026-09-10 from local source.

Read [README.md](README.md) for current execution and build constraints.

## Integration points

- `bsp/esp32c6/runtime/main/main.cpp`: HAL/board/VM startup and V4-link polling.
- `bsp/esp32c6/runtime/main/CMakeLists.txt`: direct source integration of engine, hal and link.
- `bsp/esp32c6/runtime/main/v4_link_port.cpp`: USB Serial/JTAG input and response output.
- `bsp/esp32c6/runtime/main/panic_handler.cpp`: custom engine panic diagnostics.
- `bsp/esp32c6/docker-compose.yml`: ESP-IDF v5.3 and sibling repository mounts.

The previously documented `runtime/components/v4_core` is not the integration point in this checkout.
The runtime directly compiles `task_backend_freertos.cpp`; preserve the FreeRTOS backend.

## Migration state

Current engine SYS has stack effect `( arg0 arg1 arg2 sys_id -- result )`.
It invokes a global callback registered through `v4_register_sys_handler()`.
This runtime currently has no such registration. V4-hal linkage does not provide automatic SYS dispatch.

V4-std initialization and build dependency, NanoC6 DDT provider, ESP32 LED HAL and RGB LED build entries are commented out.
RGB LED initialization in main is also commented out.
Treat these as incomplete integration, not enabled features.

The current main receives bytecode through V4-link; Forth source compilation takes place in the host CLI.
Do not document an on-device Forth prompt as the behavior of this build.

## Build constraints

Native local dependency paths in main/CMakeLists.txt resolve under V4-runtime instead of the workspace's sibling repositories.
Container paths (/v4-engine, /v4-hal, /v4-link) and runtime/_deps are alternative lookup locations.
Compose uses the container paths. Local .env settings can alter the working directory/device; do not copy their contents into shared documentation.

Preserve pre-existing local commits and untracked environment files.
Record build/test/device verification against the actual revision. Historical successes and size estimates do not validate the current migration state.

New platform integration belongs in V4-runtime; V4-ports is deprecated.
