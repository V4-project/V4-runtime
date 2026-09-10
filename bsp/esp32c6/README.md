# ESP32-C6 BSP

Board Support Package for ESP32-C6 RISC-V microcontrollers.

## Chip Specifications

- **CPU**: RISC-V 32-bit single-core @ 160 MHz
- **RAM**: 512 KB SRAM
- **Flash**: 4 MB (typical)
- **Connectivity**: WiFi 6, Bluetooth 5, IEEE 802.15.4 (Zigbee/Thread)
- **Peripherals**: GPIO, UART, SPI, I2C, ADC, PWM, USB Serial/JTAG

## Supported Boards

| Board | Manufacturer | Features | Status |
|-------|--------------|----------|--------|
| NanoC6 | M5Stack | Compact, RGB LED, Grove connector | Stable |
| ESP32-C6-DevKit | Espressif | Development board | Planned |

## Directory Structure

```
esp32c6/
├── hal_integration/        # V4 HAL → ESP-IDF bridge
│   ├── CMakeLists.txt
│   └── v4_hal_esp32c6.c
├── linker/
│   └── esp32c6.ld         # Memory layout
├── boards/
│   ├── nanoc6/            # M5Stack NanoC6
│   │   ├── board.h
│   │   ├── sdkconfig
│   │   └── README.md
│   └── devkit/            # ESP32-C6-DevKit
└── examples/
    ├── nanoc6/
    │   ├── hello-rtos/
    │   └── blink/
    └── devkit/
```

## Prerequisites

### ESP-IDF Installation

V4 RTOS for ESP32-C6 targets ESP-IDF v5.5.5 (matching CI and Docker):

```bash
# Install ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --branch v5.5.5 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Install for ESP32-C6
./install.sh esp32c6

# Source environment
. $HOME/esp/esp-idf/export.sh
```

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

## Building Examples

### NanoC6 Hello RTOS

```bash
cd bsp/esp32c6/examples/nanoc6/hello-rtos
idf.py build flash monitor
```

### Blink Example

```bash
cd bsp/esp32c6/examples/nanoc6/blink
idf.py build flash monitor
```

## Configuration

Each board has a default `sdkconfig` file. To customize:

```bash
idf.py menuconfig
```

Key configurations:

- **Component config → FreeRTOS**: Disable (V4 RTOS provides scheduler)
- **Serial flasher config**: Set baud rate, flash size
- **Partition Table**: Adjust for your application

## HAL Integration

The ESP32-C6 HAL integration bridges V4 HAL to ESP-IDF:

```c
// V4 HAL call
gpio_set_mode(GPIO_NUM_7, GPIO_MODE_OUTPUT);
gpio_write(GPIO_NUM_7, 1);

// Mapped to ESP-IDF
gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
gpio_set_level(GPIO_NUM_7, 1);
```

## Memory Map

```
0x4200_0000 ┌──────────────────┐
            │   Flash (RO)     │ 4 MB
            │   - Bootloader   │
            │   - App Code     │
            │   - Data         │
0x4240_0000 ├──────────────────┤

0x4080_0000 ┌──────────────────┐
            │   SRAM (RW)      │ 512 KB
            │   - .data        │
            │   - .bss         │
            │   - Heap         │
            │   - Stacks       │
0x4087_FFFF └──────────────────┘
```

## Debugging

### Serial Monitor

```bash
idf.py monitor
```

Exit with `Ctrl+]`.

### GDB Debugging

```bash
# Terminal 1: Start OpenOCD
openocd -f board/esp32c6-builtin.cfg

# Terminal 2: Start GDB
riscv32-esp-elf-gdb build/hello-rtos.elf
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) continue
```

## Performance

ESP32-C6 @ 160 MHz:

- V4 context switch: ~80μs
- GPIO toggle: ~500ns
- UART byte: ~8μs @ 115200

## Troubleshooting

### Build fails with "idf.py not found"

Make sure ESP-IDF is sourced:

```bash
. $HOME/esp/esp-idf/export.sh
```

### Flash fails

1. Check USB connection
2. Install drivers (Windows: CH340/CP210x)
3. Check permissions: `sudo usermod -a -G dialout $USER`
4. Try different baud rate: `idf.py -b 115200 flash`

### Board doesn't boot after flash

1. Check power supply
2. Erase flash: `idf.py erase-flash`
3. Reflash bootloader and app

## See Also

- [Getting Started Guide](../../docs/getting-started.md)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-C6 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c6_datasheet_en.pdf)
