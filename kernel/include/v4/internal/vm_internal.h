#pragma once

/**
 * @file vm_internal.h
 * @brief V4-RTOS kernel internal API
 *
 * V4-RTOS uses V4 VM's built-in scheduler and message queue.
 * This header exposes V4 APIs and provides thin wrappers.
 */

#include <v4/errors.h>
#include <v4/task.h>
#include <v4/vm_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Create and initialize V4 VM with RTOS support
   *
   * This is a thin wrapper around vm_create() and vm_task_init().
   * Returns a V4 Vm instance with scheduler and message queue initialized.
   */
  struct Vm *v4_rtos_vm_create(const VmConfig *cfg);

  /**
   * @brief Destroy V4 VM and cleanup tasks
   *
   * Wrapper around vm_task_cleanup() and vm_destroy().
   */
  void v4_rtos_vm_destroy(struct Vm *vm);

  /*
   * V4-RTOS uses V4's task/scheduler/message APIs directly:
   *
   * Task API (v4/task.h):
   *   - vm_task_init()       - Initialize task system
   *   - vm_task_spawn()      - Spawn new task
   *   - vm_task_exit()       - Exit current task
   *   - vm_task_yield()      - Yield to scheduler
   *   - vm_task_sleep()      - Sleep for milliseconds
   *   - vm_task_self()       - Get current task ID
   *   - vm_task_get_info()   - Get task state/priority
   *   - vm_task_send()       - Send message to task
   *   - vm_task_receive()    - Receive message (blocking/non-blocking)
   *
   * Scheduler API (v4/internal/vm.h):
   *   - vm_schedule()        - Schedule next task
   *   - v4_task_select_next() - Select next ready task
   */

#ifdef __cplusplus
}
#endif
