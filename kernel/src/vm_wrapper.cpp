#include "v4/errors.hpp"
#include "v4/internal/vm.h"
#include "v4/internal/vm_internal.h"

/**
 * @file vm_wrapper.cpp
 * @brief V4-RTOS VM wrapper
 *
 * Thin C++ wrapper around V4 VM to initialize RTOS features.
 * V4 VM already includes scheduler and message queue.
 */

extern "C" Vm *v4_rtos_vm_create(const VmConfig *cfg)
{
  if (!cfg)
    return nullptr;

  // Create V4 VM instance
  Vm *vm = vm_create(cfg);
  if (!vm)
    return nullptr;

  // Initialize task scheduler (10ms time slice)
  v4_err err = vm_task_init(vm, 10);
  if (err != V4_ERR(OK))
  {
    vm_destroy(vm);
    return nullptr;
  }

  return vm;
}

extern "C" void v4_rtos_vm_destroy(Vm *vm)
{
  if (!vm)
    return;

  // Cleanup all tasks (frees task stacks)
  vm_task_cleanup(vm);

  // Destroy VM
  vm_destroy(vm);
}
