#include <stdlib.h>
#include <string.h>

#include "v4/internal/vm_internal.h"

v4_err v4_task_spawn(v4_vm_t *vm, uint16_t word_idx, uint8_t priority, uint8_t ds_size,
                     uint8_t rs_size)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_scheduler_t *sched = &vm->scheduler;

  /* Find empty slot */
  int slot = -1;
  for (int i = 0; i < V4_MAX_TASKS; i++)
  {
    if (sched->tasks[i].state == V4_TASK_STATE_DEAD)
    {
      slot = i;
      break;
    }
  }

  if (slot < 0)
    return V4_ERR_TASK_LIMIT;

  v4_task_t *task = &sched->tasks[slot];

  /* Allocate independent stacks */
  task->ds_base = (v4_i32 *)malloc(ds_size * sizeof(v4_i32));
  task->rs_base = (v4_i32 *)malloc(rs_size * sizeof(v4_i32));

  if (!task->ds_base || !task->rs_base)
  {
    free(task->ds_base);
    free(task->rs_base);
    return V4_ERR_NO_MEMORY;
  }

  /* Initialize task */
  task->word_idx = word_idx;
  task->pc = 0;
  task->ds_depth = 0;
  task->rs_depth = 0;
  task->ds_size = ds_size;
  task->rs_size = rs_size;
  task->state = V4_TASK_STATE_READY;
  task->priority = priority;
  task->sleep_until_tick = 0;
  task->exec_count = 0;

  sched->task_count++;

  return V4_OK;
}

v4_err v4_task_yield(v4_vm_t *vm)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  return v4_schedule(vm);
}

v4_err v4_task_sleep(v4_vm_t *vm, v4_u32 ms)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_scheduler_t *sched = &vm->scheduler;
  v4_task_t *task = &sched->tasks[sched->current_task];

  v4_u32 current_tick = v4_platform_get_tick_ms();
  task->sleep_until_tick = current_tick + ms;
  task->state = V4_TASK_STATE_BLOCKED;

  /* Trigger reschedule */
  return v4_schedule(vm);
}

v4_err v4_task_exit(v4_vm_t *vm)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_scheduler_t *sched = &vm->scheduler;
  v4_task_t *task = &sched->tasks[sched->current_task];

  /* Free task stacks */
  if (task->ds_base)
  {
    free(task->ds_base);
    task->ds_base = NULL;
  }
  if (task->rs_base)
  {
    free(task->rs_base);
    task->rs_base = NULL;
  }

  task->state = V4_TASK_STATE_DEAD;
  sched->task_count--;

  /* Trigger reschedule */
  return v4_schedule(vm);
}

uint8_t v4_task_self(v4_vm_t *vm)
{
  if (!vm)
    return 0;

  return vm->scheduler.current_task;
}

uint8_t v4_task_count(v4_vm_t *vm)
{
  if (!vm)
    return 0;

  return vm->scheduler.task_count;
}
