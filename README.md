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
- The panic handler provides ESP logging, LED indication and a halt loop.
- V4-std initialization, NanoC6 DDT provider and LED HAL build integration are commented out.
- RGB LED driver sources exist, but their build entries and initialization are commented out.
- The current engine requires a global `v4_register_sys_handler()` callback for SYS calls. This runtime does not register it yet.
  Linking V4-hal alone does not connect SYS to GPIO or timers.
- Examples using GPIO-WRITE, DELAY or string-output words require work before they can run with the current compiler/library.
- OTA, JIT and additional MCU support remain plans.

## Source map

| Location | Purpose |
|---|---|
| [runtime/main/main.cpp](bsp/esp32c6/runtime/main/main.cpp) | Startup and V4-link polling |
| [runtime/main/CMakeLists.txt](bsp/esp32c6/runtime/main/CMakeLists.txt) | Direct compilation of engine, hal and link sources |
| [runtime/main/v4_link_port.cpp](bsp/esp32c6/runtime/main/v4_link_port.cpp) | USB Serial/JTAG transport |
| [runtime/main/panic_handler.cpp](bsp/esp32c6/runtime/main/panic_handler.cpp) | Panic diagnostics |
| [boards/nanoc6](bsp/esp32c6/boards/nanoc6/) | Board setup and DDT provider |
| [hal_esp32](bsp/esp32c6/hal_esp32/) | LED HAL and RGB LED sources |
| [docker-compose.yml](bsp/esp32c6/docker-compose.yml) | ESP-IDF v5.3 development environment |

The ESP-IDF main component directly includes `task_backend_freertos.cpp`; it does not select CUSTOM.

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
