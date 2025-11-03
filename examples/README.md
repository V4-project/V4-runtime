# V4 RTOS Customization Examples

Examples demonstrating how to customize and extend V4 RTOS itself.

## Purpose

This directory contains examples of **modifying the RTOS**, not user applications.

**For user applications (Forth programs)**, see `tools/examples/`.

**For the runtime to flash to device**, see `bsp/esp32c6/runtime/`.

## What Goes Here

Examples of customizing V4 RTOS:

- Custom scheduler implementations
- Additional system calls
- Memory management strategies
- HAL extensions for new peripherals
- Custom BSP implementations

## Planned Examples

### Custom Scheduler (Planned)

Demonstrates implementing a priority-based scheduler:

```c
// examples/custom-scheduler/priority_scheduler.c
void schedule_next_task(void) {
    // Find highest priority ready task
    task_t *next = find_highest_priority_ready();
    context_switch(next);
}
```

### Custom System Call (Planned)

Adding a new SYS instruction:

```c
// examples/custom-syscall/README.md
// How to add SYS 100 for custom functionality
```

### Memory Pool (Planned)

Implementing a memory pool allocator:

```c
// examples/memory-pool/pool_allocator.c
void* pool_alloc(size_t size);
void pool_free(void* ptr);
```

## NOT Here

- **Forth user programs** → See `tools/examples/`
- **Hardware-specific examples** → See `bsp/<platform>/`
- **Runtime application** → See `bsp/esp32c6/runtime/`

## Development Status

Currently placeholders. Will be populated as the RTOS matures.

## Contributing

If you create a useful RTOS customization, please consider contributing it as an example!

## See Also

- [Architecture Documentation](../docs/architecture.md)
- [Kernel Source](../kernel/)
- [HAL Source](../hal/)
- [Forth Examples](../tools/examples/)
