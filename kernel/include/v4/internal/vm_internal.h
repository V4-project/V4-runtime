#pragma once
#include "v4/task.h"
#include "v4/types.h"
#include "v4/vm.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @file vm_internal.h
   * @brief Internal VM data structures
   *
   * Not part of the public API.
   */

#define V4_MAX_WORDS 256
#define V4_DS_SIZE 256
#define V4_RS_SIZE 64

  /**
   * @brief Word entry
   */
  typedef struct v4_word
  {
    char *name;
    const v4_u8 *code;
    int code_len;
  } v4_word_t;

  /**
   * @brief VM instance
   */
  typedef struct v4_vm
  {
    /* Stacks */
    v4_i32 DS[V4_DS_SIZE];
    v4_i32 RS[V4_RS_SIZE];
    v4_i32 *sp; /* Data stack pointer */
    v4_i32 *rp; /* Return stack pointer */
    v4_i32 *fp; /* Frame pointer (locals) */

    /* Memory */
    v4_u8 *mem;
    v4_u32 mem_size;

    /* Dictionary */
    v4_word_t words[V4_MAX_WORDS];
    int word_count;

    /* Task system */
    v4_scheduler_t scheduler;
    v4_msg_queue_t msg_queue;

    /* Execution state */
    int last_err;
  } v4_vm_t;

  /* Internal stack operations */
  v4_err v4_ds_push_internal(v4_vm_t *vm, v4_i32 val);
  v4_err v4_ds_pop_internal(v4_vm_t *vm, v4_i32 *out);
  v4_err v4_ds_peek_internal(const v4_vm_t *vm, int idx, v4_i32 *out);
  v4_err v4_rs_push_internal(v4_vm_t *vm, v4_i32 val);
  v4_err v4_rs_pop_internal(v4_vm_t *vm, v4_i32 *out);

  /* Scheduler interface */
  void v4_scheduler_init(v4_scheduler_t *sched);
  v4_err v4_schedule(v4_vm_t *vm);
  uint8_t v4_task_select_next(v4_vm_t *vm);
  v4_err v4_task_spawn(v4_vm_t *vm, uint16_t word_idx, uint8_t priority, uint8_t ds_size,
                       uint8_t rs_size);
  v4_err v4_task_yield(v4_vm_t *vm);
  v4_err v4_task_sleep(v4_vm_t *vm, v4_u32 ms);
  v4_err v4_task_exit(v4_vm_t *vm);
  uint8_t v4_task_self(v4_vm_t *vm);
  uint8_t v4_task_count(v4_vm_t *vm);

  /* Message queue interface */
  void v4_msg_queue_init(v4_msg_queue_t *q);
  v4_err v4_msg_send(v4_vm_t *vm, uint8_t dst_task, uint8_t msg_type, v4_i32 data);
  v4_err v4_msg_receive(v4_vm_t *vm, uint8_t msg_type, v4_i32 *data, uint8_t *src_task,
                        int blocking, v4_u32 timeout_ms);

  /* Platform time interface (must be provided by BSP) */
  v4_u32 v4_platform_get_tick_ms(void);
  v4_u32 v4_platform_get_tick_us(void);
  void v4_platform_delay_ms(v4_u32 ms);
  void v4_platform_delay_us(v4_u32 us);

#ifdef __cplusplus
} /* extern "C" */
#endif
