# V4 RTOS

**Embedded Forth Real-Time Operating System**

[![CI](https://github.com/V4-project/V4-rtos/workflows/CI/badge.svg)](https://github.com/V4-project/V4-rtos/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

V4 RTOS is a lightweight, preemptive real-time operating system for resource-constrained microcontrollers, featuring interactive Forth development.

## Features

### Core (Required)

- **Preemptive Multitasking** - 8 tasks, <100μs context switch
- **Message Passing** - Inter-task communication with 16-message queue
- **Hardware Abstraction** - Unified HAL across platforms

### Optional Components

- **Forth Compiler** - Compile Forth source to V4 bytecode
- **Interactive REPL** - Live Forth programming on device
- **Protocol Stack** - V4-link for OTA updates
- **JIT Compilation** - Runtime optimization (planned)

## Quick Start (10 minutes)

```bash
# 1. Clone repository
git clone https://github.com/V4-project/V4-rtos.git
cd V4-rtos

# 2. Flash V4 runtime to ESP32-C6
cd bsp/esp32c6/runtime
idf.py build flash monitor

# 3. Compile and send Forth program (when v4_cli is ready)
v4c tools/examples/hello.fth -o hello.bin
v4flash -p /dev/ttyUSB0 hello.bin
```

See [Getting Started Guide](docs/getting-started.md) for detailed instructions.

## Architecture

```
V4 RTOS
├── kernel/      VM core + scheduler (42KB) [required]
├── hal/         Hardware abstraction (5.7KB) [required]
├── compiler/    Forth → bytecode [optional]
├── shell/       Interactive REPL (91KB) [optional]
├── protocol/    V4-link protocol (1.5KB) [optional]
├── bsp/         Board support packages
└── tools/       Development CLI
```

**Minimum Configuration**: kernel + hal (~48KB)
**Full Configuration**: All components (~140KB)

## Supported Platforms

| Platform | Status | MCU | Arch | RAM | Flash |
|----------|--------|-----|------|-----|-------|
| **ESP32-C6** | Stable | ESP32-C6 | RISC-V 32 | 512KB | 4MB |
| ESP32-S3 | Planned | ESP32-S3 | Xtensa LX7 | 512KB | 8MB |
| CH32V203 | Planned | CH32V203 | RISC-V 32 | 20KB | 64KB |
| RP2350 | Planned | RP2350 | ARM + RISC-V | 520KB | 4MB |

## Comparison with Other RTOSes

| Feature | V4 RTOS | FreeRTOS | Zephyr | Mecrisp |
|---------|---------|----------|--------|---------|
| **Scheduler** | Preemptive | Preemptive | Preemptive | Cooperative |
| **Language** | Forth | C | C | Forth |
| **VM** | Yes | No | No | No |
| **REPL** | Yes | No | No | Yes |
| **Flash** | 64KB~ | 10KB~ | 256KB~ | 16KB~ |
| **Context Switch** | <100μs | 1-5μs | 2-10μs | N/A |
| **OTA** | Planned | Manual | Yes | Manual |
| **JIT** | Planned | No | No | No |

**V4 RTOS Advantages:**
- Only Forth RTOS with preemptive multitasking
- Interactive development without reflashing
- Designed for hot-swapping and live updates
- Unified VM/REPL/RTOS integration

## Documentation

- [Getting Started](docs/getting-started.md) - 10-minute tutorial
- [Architecture](docs/architecture.md) - System design overview
- [API Reference](docs/api-reference/) - Complete API documentation
- [Porting Guide](docs/porting-guide.md) - Adding new platforms
- [Building Guide](docs/building.md) - Build system reference

## Building

```bash
# Build all components
make build

# Run tests
make test

# Format code
make format

# Build ESP32-C6 examples
cd bsp/esp32c6/examples/nanoc6/hello-rtos
idf.py build
```

See [Building Guide](docs/building.md) for detailed instructions.

## Components

### Core Components (Required)

- **[kernel/](kernel/)** - VM core + preemptive scheduler
- **[hal/](hal/)** - C++17 CRTP hardware abstraction
- **[bsp/](bsp/)** - Board support packages

### Optional Components

- **[compiler/](compiler/)** - Forth-to-bytecode compiler
- **[shell/](shell/)** - Interactive REPL
- **[protocol/](protocol/)** - V4-link bytecode transfer
- **[tools/](tools/)** - CLI development tools

## Contributing

Contributions are welcome! Currently in active development - please open an issue to discuss before submitting pull requests.

## License

Dual-licensed under:
- MIT License ([LICENSE-MIT](LICENSE-MIT))
- Apache License 2.0 ([LICENSE-APACHE](LICENSE-APACHE))

Choose either license for your use.

## Links

- [Documentation](docs/)
- [Issue Tracker](https://github.com/V4-project/V4-rtos/issues)
- [Discussions](https://github.com/V4-project/V4-rtos/discussions)
