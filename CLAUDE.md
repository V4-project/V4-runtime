# Claude Code Notes

## V4-runtime Overview

V4-runtime is the **active runtime environment** for V4 Forth VM on embedded platforms, specifically designed for FreeRTOS-based systems.

### Key Information

- **Status**: Active development (replaces V4-ports)
- **Target**: ESP32-C6 and other FreeRTOS-capable MCUs
- **Backend**: FreeRTOS task scheduler (not custom V4 scheduler)

## V4-engine Task Backend Integration (2025-11-04)

### Architecture Changes

V4-engine now has a **pluggable task backend** system:

1. **Custom Backend** (`V4_TASK_BACKEND=CUSTOM`)
   - V4's own priority + round-robin scheduler
   - Used for: Desktop, testing, non-RTOS platforms
   - Implementation: `V4-engine/src/task_backend_custom.cpp`

2. **FreeRTOS Backend** (`V4_TASK_BACKEND=FREERTOS`)
   - Native FreeRTOS task integration
   - Used for: ESP32, ESP-IDF, and other FreeRTOS platforms
   - Implementation: `V4-engine/src/task_backend_freertos.cpp`
   - ESP-IDF compatible: Uses `freertos/FreeRTOS.h` when `ESP_PLATFORM` is defined

### Integration with V4-runtime

When building V4-runtime for ESP32-C6:

```cmake
# In ESP-IDF component (e.g., components/v4_core/CMakeLists.txt)
idf_component_register(
  SRCS
    "${V4_DIR}/src/core.cpp"
    "${V4_DIR}/src/memory.cpp"
    "${V4_DIR}/src/arena.cpp"
    "${V4_DIR}/src/task.cpp"
    "${V4_DIR}/src/scheduler.cpp"
    "${V4_DIR}/src/message.cpp"
    "${V4_DIR}/src/task_backend_freertos.cpp"  # Use FreeRTOS backend
  REQUIRES
    v4_hal
    freertos  # ESP-IDF FreeRTOS component
)
```

### Files to Update

When integrating the new task backend system:

✅ **Required**:
- `bsp/esp32c6/runtime/components/v4_core/CMakeLists.txt` - Add task backend sources
- Update to use `V4-engine` (not old `V4` repo name)

⚠️ **Important**:
- V4-runtime **ALWAYS** uses `FREERTOS` backend
- Do NOT use `CUSTOM` backend for V4-runtime
- FreeRTOS is provided by ESP-IDF (no manual FreeRTOS setup needed)

### Backend API

The backend interface is defined in `V4-engine/include/v4/internal/task_backend.h`:

```c
// Task management
v4_err v4_backend_task_init(Vm *vm, uint32_t time_slice_ms);
v4_err v4_backend_task_cleanup(Vm *vm);
int v4_backend_task_spawn(Vm *vm, uint16_t word_idx, uint8_t priority, ...);
v4_err v4_backend_task_exit(Vm *vm);
v4_err v4_backend_task_sleep(Vm *vm, uint32_t ms_delay);
v4_err v4_backend_task_yield(Vm *vm);

// Message passing
v4_err v4_backend_task_send(Vm *vm, uint8_t dst_task, ...);
int v4_backend_task_receive(Vm *vm, uint8_t msg_type, ...);
int v4_backend_task_receive_blocking(Vm *vm, uint8_t msg_type, ...);

// Scheduling
v4_err v4_backend_schedule(Vm *vm);
v4_err v4_backend_schedule_from_isr(Vm *vm);
```

### Priority Mapping

V4 uses 8-bit priority (0-255), FreeRTOS uses `configMAX_PRIORITIES`:

```c
// In task_backend_freertos.cpp
#define V4_TO_FREERTOS_PRIORITY(v4_prio) \
  ((v4_prio * (configMAX_PRIORITIES - 1)) / 255)
```

## Repository Structure

```
V4-runtime/
├── bsp/
│   └── esp32c6/
│       ├── runtime/          ← Main runtime application
│       │   └── components/
│       │       └── v4_core/  ← V4-engine integration point
│       ├── boards/           ← Board configs (NanoC6, DevKit)
│       └── hal_integration/  ← V4-hal wrapper
├── hal/                      ← V4-hal CMake wrapper
├── tools/                    ← Examples and utilities
└── docs/                     ← Documentation
```

## Related Repositories

- **V4-engine**: https://github.com/V4-project/V4-engine
  - VM core with task backend abstraction
  - Use with `V4_TASK_BACKEND=FREERTOS` for ESP32

- **V4-hal**: https://github.com/V4-project/V4-hal
  - Hardware abstraction layer
  - Platform-independent HAL API

- **V4-front**: https://github.com/kirisaki/V4-front
  - Forth compiler (Forth → V4 bytecode)

- **V4-ports**: https://github.com/kirisaki/V4-ports
  - **DEPRECATED** - Use V4-runtime instead

## Migration from V4-ports

If you see references to V4-ports:

1. V4-ports is deprecated and being archived
2. All new development uses V4-runtime
3. V4-runtime has better FreeRTOS integration
4. Task backend is now abstracted in V4-engine

## V4-engine Panic Handler Integration (2025-11-05)

### Overview

V4-runtime now integrates V4-engine v0.12.0's panic handler API for comprehensive error reporting on ESP32-C6.

### Implementation

**Location**: `bsp/esp32c6/runtime/main/panic_handler.cpp`

**Features**:
- Detailed panic information logging via ESP_LOGE
- Formatted output with error code, task ID, instruction pointer
- Stack state display (depth and top 4 values)
- Visual indication via rapid LED blinking (100ms on/off)
- System halt for safe debugging

**Integration**:
```cpp
// In main.cpp, after vm_create()
g_vm = vm_create(&config);
panic_handler_init(g_vm);  // Register panic handler
```

**Panic Output Example**:
```
E (1234) v4-panic: ╔═══════════════════════════════════════╗
E (1234) v4-panic: ║  V4 VM PANIC - FATAL ERROR           ║
E (1234) v4-panic: ╚═══════════════════════════════════════╝
E (1234) v4-panic: Error Code:    -3 (STACK_OVERFLOW)
E (1234) v4-panic: Task ID:       0
E (1234) v4-panic: IP (Instr Ptr): 0x00000042
E (1234) v4-panic: Message:       Data stack overflow
E (1234) v4-panic: Stack Depth:   256 / 256
E (1234) v4-panic: Return Depth:  12 / 64
E (1234) v4-panic: Stack Values:
E (1234) v4-panic:   [0]: 0x00001234 (4660)
E (1234) v4-panic:   [1]: 0xFFFFFFFF (-1)
E (1234) v4-panic: System halted. Reset required.
```

### CMakeLists.txt Configuration

Added to `bsp/esp32c6/runtime/main/CMakeLists.txt`:
```cmake
# Include panic.cpp from V4-engine
set(V4_SRCS
    ...
    "${V4_DIR}/src/panic.cpp"
    ...
)

# Include panic_handler.cpp in component
idf_component_register(
  SRCS
    "panic_handler.cpp"
    ...
)
```

## Last Updated

2025-11-05: Added panic handler integration notes
