# V4 RTOS Runtime for ESP32-C6

This is the V4 RTOS runtime application that runs on ESP32-C6 (M5Stack NanoC6).

## Purpose

The runtime provides:

- **V4 Virtual Machine** - Executes V4 bytecode
- **Preemptive Scheduler** - Manages up to 8 concurrent tasks
- **Message Passing** - Inter-task communication
- **Bytecode Reception** - Receives code via UART using V4-link protocol

**Flash this once to your device**, then send Forth programs as bytecode from your host computer.

## Building and Flashing

### Prerequisites

- ESP-IDF v5.1 or later
- ESP32-C6 board (M5Stack NanoC6 recommended)

### Setup

```bash
# Source ESP-IDF
. $HOME/esp/esp-idf/export.sh

# Navigate to runtime
cd bsp/esp32c6/runtime
```

### Build

```bash
idf.py build
```

### Flash

```bash
idf.py flash
```

### Monitor

```bash
idf.py monitor
```

Expected output:

```
I (123) v4-runtime: === V4 RTOS Runtime ===
I (124) v4-runtime: Board: M5Stack NanoC6
I (125) v4-runtime: Version: 1.0.0-dev
I (126) v4-runtime: Board initialized
I (127) v4-runtime: V4 VM initialized
I (128) v4-runtime: Runtime ready. Send bytecode via UART.
I (129) v4-runtime: Use: v4flash -p /dev/ttyUSB0 program.bin
I (130) v4-runtime: Waiting for bytecode...
```

## Usage Workflow

Once the runtime is flashed:

### 1. Write Forth Code

Create a Forth source file (e.g., `hello.fth`):

```forth
: HELLO  ." Hello from V4 RTOS!" CR ;
HELLO
```

### 2. Compile to Bytecode

Using v4_cli (when implemented):

```bash
v4c hello.fth -o hello.bin
```

### 3. Send Bytecode to Device

```bash
v4flash -p /dev/ttyUSB0 hello.bin
```

### 4. View Output

The runtime receives, loads, and executes the bytecode:

```
I (1234) v4-runtime: Received 128 bytes
I (1235) v4-runtime: Executing bytecode (128 bytes)
Hello from V4 RTOS!
I (1236) v4-runtime: Execution complete
I (1237) v4-runtime: Waiting for bytecode...
```

## Development Status

**Current (v1.0.0-dev)**:
- Basic structure and UART reception
- Placeholder VM (echoes received bytes)
- LED status indication

**Planned**:
- Actual V4 VM implementation (kernel integration)
- V4-link protocol for reliable transfer
- Preemptive scheduler integration
- Message passing between tasks
- Error handling and recovery

## Architecture

```
┌─────────────────────────────────┐
│     Host Computer (v4_cli)      │
│  hello.fth → v4c → hello.bin    │
└──────────────┬──────────────────┘
               │ UART (V4-link)
               ▼
┌─────────────────────────────────┐
│   ESP32-C6 (V4 Runtime)         │
│  ┌───────────────────────────┐  │
│  │   Bytecode Reception      │  │
│  └────────────┬──────────────┘  │
│               ▼                  │
│  ┌───────────────────────────┐  │
│  │      V4 VM Engine         │  │
│  │   (Stack-based VM)        │  │
│  └────────────┬──────────────┘  │
│               ▼                  │
│  ┌───────────────────────────┐  │
│  │   Preemptive Scheduler    │  │
│  │   (8 tasks, <100μs ctx)   │  │
│  └────────────┬──────────────┘  │
│               ▼                  │
│  ┌───────────────────────────┐  │
│  │         HAL Layer         │  │
│  │  (GPIO, UART, Timer...)   │  │
│  └───────────────────────────┘  │
└─────────────────────────────────┘
```

## Memory Usage

- **Flash**: ~48 KB (minimal kernel + HAL)
- **RAM**: ~8.5 KB base + 8 KB per task
- **Bytecode Buffer**: 4 KB (configurable)

## Customization

### Change Bytecode Buffer Size

Edit `main.c`:

```c
typedef struct {
    uint8_t bytecode[8192];  // Increase to 8 KB
    // ...
} v4_vm_t;
```

### Add Custom System Calls

Implement new syscalls in the VM execution loop (when kernel is integrated).

### Configure Board

Edit `../boards/nanoc6/board.h` for pin assignments.

## Troubleshooting

### Runtime doesn't start

- Check power supply
- Verify ESP-IDF is sourced
- Try: `idf.py erase-flash` then reflash

### Bytecode not received

- Check UART connection
- Verify baud rate (default: 115200)
- Ensure v4_cli is using correct port

### LED doesn't turn on

- Verify board is NanoC6 (LED on GPIO 7)
- Check board.h pin configuration

## See Also

- [V4 RTOS Architecture](../../../docs/architecture.md)
- [NanoC6 Board Configuration](../boards/nanoc6/README.md)
- [V4 CLI Tools](../../../tools/README.md)
- [V4-link Protocol](../../../protocol/README.md)
