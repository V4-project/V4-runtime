#include <cstdio>
#include <cstring>

#include "v4/errors.hpp"
#include "v4/internal/vm.h"
#include "v4/internal/vm_internal.h"

// Simple test runner
static int test_count = 0;
static int test_passed = 0;

#define TEST_ASSERT(cond, msg)                                        \
  do                                                                  \
  {                                                                   \
    test_count++;                                                     \
    if (!(cond))                                                      \
    {                                                                 \
      fprintf(stderr, "FAIL: %s:%d - %s\n", __FILE__, __LINE__, msg); \
      return 1;                                                       \
    }                                                                 \
    test_passed++;                                                    \
  } while (0)

static int test_vm_create_destroy()
{
  printf("Testing VM create/destroy...\n");

  // Setup VM config
  uint8_t ram[256];
  VmConfig cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.mem = ram;
  cfg.mem_size = sizeof(ram);
  cfg.mmio = nullptr;
  cfg.mmio_count = 0;

  // Create RTOS VM (returns V4 Vm with scheduler initialized)
  Vm *vm = v4_rtos_vm_create(&cfg);
  TEST_ASSERT(vm != nullptr, "v4_rtos_vm_create should succeed");

  // Now we can access Vm internals directly (C++)
  TEST_ASSERT(vm->scheduler.current_task == 0, "Scheduler should start at task 0");
  TEST_ASSERT(vm->scheduler.task_count == 0, "No tasks initially");
  TEST_ASSERT(vm->scheduler.time_slice_ms == 10, "Time slice should be 10ms");

  // Check all tasks are dead
  for (int i = 0; i < V4_MAX_TASKS; i++)
  {
    TEST_ASSERT(vm->scheduler.tasks[i].state == V4_TASK_STATE_DEAD,
                "All tasks should be DEAD initially");
  }

  // Check message queue is initialized
  TEST_ASSERT(vm->msg_queue.count == 0, "Message queue should be empty");
  TEST_ASSERT(vm->msg_queue.read_idx == 0, "Message queue read index should be 0");
  TEST_ASSERT(vm->msg_queue.write_idx == 0, "Message queue write index should be 0");

  // Destroy
  v4_rtos_vm_destroy(vm);

  return 0;
}

static int test_vm_create_null_config()
{
  printf("Testing VM create with NULL config...\n");

  Vm *vm = v4_rtos_vm_create(nullptr);
  TEST_ASSERT(vm == nullptr, "v4_rtos_vm_create with NULL config should fail");

  return 0;
}

static int test_vm_destroy_null()
{
  printf("Testing VM destroy with NULL...\n");

  // Should not crash
  v4_rtos_vm_destroy(nullptr);
  TEST_ASSERT(true, "v4_rtos_vm_destroy with NULL should not crash");

  return 0;
}

int main(void)
{
  printf("=== VM Wrapper Tests ===\n");

  if (test_vm_create_destroy() != 0)
    return 1;
  if (test_vm_create_null_config() != 0)
    return 1;
  if (test_vm_destroy_null() != 0)
    return 1;

  printf("\n%d/%d tests passed\n", test_passed, test_count);
  return 0;
}
