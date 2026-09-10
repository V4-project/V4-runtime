# V4 Runtime

FreeRTOS-based runtime integrating V4-engine, V4-link and V4-hal on ESP32-C6 / M5Stack NanoC6.
It replaces the deprecated V4-ports implementation.

Status checked against local source on 2026-09-10. Historical hardware demonstrations are not a validation of the current checkout.

## Current execution path

```text
Host:   v4_cli → V4-front → bytecode
                             ↓ USB Serial/JTAG, V4-link frames
Device: runtime main → V4-link → V4-engine / FreeRTOS
```

The current firmware polls for binary V4-link frames. It does not include V4-front or an on-device text REPL.
Use the host CLI's `v4 repl --port /dev/ttyACM0` to enter Forth source.

## Implemented and incomplete parts

- VM initialization, FreeRTOS task backend, board initialization and V4-link reception are present.
- The panic handler uses engine error messages, logs diagnostics and lights the LED, then returns so V4-link can report the failure. Execution errors do not roll back VM state; inspect it or RESET before retrying.
- V4-std initialization, NanoC6 DDT provider and LED HAL build integration are commented out.
- RGB LED driver sources exist, but their build entries and initialization are commented out.
- The current engine requires a global `v4_register_sys_handler()` callback for SYS calls. This runtime does not register it yet.
  Linking V4-hal alone does not connect SYS to GPIO or timers.
- Examples using GPIO-WRITE, DELAY or string-output words require work before they can run with the current compiler/library.
- OTA, JIT and additional MCU support remain plans.

## Source map

For reproducible ESP32-C6 firmware measurements and before/after comparisons, see
[firmware size tooling](tools/size/README.md). This builds without hardware and does
not change runtime feature settings.

| Location | Purpose |
|---|---|
| [runtime/main/main.cpp](bsp/esp32c6/runtime/main/main.cpp) | Startup and V4-link polling |
| [runtime/main/CMakeLists.txt](bsp/esp32c6/runtime/main/CMakeLists.txt) | Direct compilation of engine, hal and link sources |
| [runtime/main/v4_link_port.cpp](bsp/esp32c6/runtime/main/v4_link_port.cpp) | USB Serial/JTAG transport |
| [runtime/main/panic_handler.cpp](bsp/esp32c6/runtime/main/panic_handler.cpp) | Panic diagnostics |
| [boards/nanoc6](bsp/esp32c6/boards/nanoc6/) | Board setup and DDT provider |
| [hal_esp32](bsp/esp32c6/hal_esp32/) | LED HAL and RGB LED sources |
| [docker-compose.yml](bsp/esp32c6/docker-compose.yml) | ESP-IDF v5.5.5 development environment |

The ESP-IDF main component directly includes `task_backend_freertos.cpp`; it does not select CUSTOM.

ESP-IDF is pinned to **5.5.5**. CI builds the following dependency revisions; use
the same commits in sibling repositories when reproducing the Docker build:

| Dependency | Commit |
|---|---|
| V4-engine | `a7eb42611170091c72b7a17799b57c13f62fa6d1` |
| V4-hal | `d36a55ac4782eed526c03afde9236c53f5bc84fa` |
| V4-link | `d155eefabda98ca6716cbe0f81dd8e602d41c990` |

V4-std is not compiled by the current ESP32-C6 component. Task support and standard
engine panic diagnostics remain enabled; this SDK migration does not opt out of them.
The default remains ON, but `idf.py -DV4_PANIC_DIAGNOSTICS=OFF reconfigure` can now
omit engine's standard formatter while retaining the runtime ESP_LOG/LED callback.
See [size tooling](tools/size/README.md#standard-panic-output-opt-out) for isolated ON/OFF measurements.
Hardware testing is still pending. Successful compilation is not confirmation of
boot, USB transport, scheduler or panic/LED behavior on the NanoC6.
See [ESP-IDF 5.5.5 validation](bsp/esp32c6/IDF-VALIDATION.md) for the SDK comparison,
known warnings and configuration caveats.

## Build and device workflow

The Compose definition mounts sibling V4-engine, V4-hal and V4-link repositories at the paths used by the main component:

```bash
cd bsp/esp32c6
docker compose run --rm esp-idf
# Inside the container, at the default runtime working directory:
idf.py build
```

Compose expects a serial device (default /dev/ttyACM0). PROJECT_DIR and ESP_DEVICE can change the working directory and device.
Inspect local configuration before use.

Native builds currently have a path mismatch: the main component's local fallback resolves to
V4-runtime/V4-engine (and similarly for hal/link), not the sibling repositories in the workspace.
It also searches runtime/_deps and the container mount paths. Prepare a supported layout or fix the build paths before relying on native `idf.py build`.

After a successful build, flash the intended board:

```bash
idf.py -p /dev/ttyACM0 flash
```

On the host, with the serial monitor closed:

```bash
v4 ping --port /dev/ttyACM0
v4 repl --port /dev/ttyACM0
v4 push app.v4b --port /dev/ttyACM0
```

The receiver is initialized with a 512-byte buffer setting. Arbitrary-size or chunked deployment is not established by this workflow.
Flash/RAM totals must be measured for the selected build; earlier estimates are not current guarantees.

See [CLAUDE.md](CLAUDE.md) for maintenance notes. Older detailed guides and examples may describe the pre-migration design.
