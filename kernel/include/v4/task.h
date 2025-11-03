#pragma once
#include "v4/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file task.h
 * @brief V4 RTOS Task Management API
 */

/* Task configuration */
#define V4_MAX_TASKS 8
#define V4_MSG_QUEUE_SIZE 16

  /* Task states */
  typedef enum
  {
    V4_TASK_STATE_DEAD = 0,    /* Not initialized */
    V4_TASK_STATE_READY = 1,   /* Ready to run */
    V4_TASK_STATE_RUNNING = 2, /* Currently executing */
    V4_TASK_STATE_BLOCKED = 3, /* Sleeping or waiting */
  } v4_task_state_t;

  /**
   * @brief Task Control Block (TCB)
   */
  typedef struct
  {
    /* Execution context */
    uint16_t word_idx; /* Word index to execute */
    uint16_t pc;       /* Program counter (bytecode offset) */
    v4_i32 *ds_base;   /* Data stack base */
    v4_i32 *rs_base;   /* Return stack base */
    uint8_t ds_depth;  /* Data stack depth */
    uint8_t rs_depth;  /* Return stack depth */

    /* Task state */
    uint8_t state;           /* v4_task_state_t */
    uint8_t priority;        /* 0=lowest, 255=highest */
    v4_u32 sleep_until_tick; /* Wake-up time (tick) */

    /* Stack configuration */
    uint8_t ds_size; /* DS capacity */
    uint8_t rs_size; /* RS capacity */

    /* Statistics */
    uint16_t exec_count;
    uint8_t reserved[2];
  } v4_task_t;

  /**
   * @brief Task Scheduler
   */
  typedef struct
  {
    v4_task_t tasks[V4_MAX_TASKS];
    uint8_t current_task;
    uint8_t task_count;
    v4_u32 tick_count;
    v4_u32 time_slice_ms;
    v4_u32 context_switches;
    v4_u32 preemptions;
    uint8_t critical_nesting;
    uint8_t reserved[3];
  } v4_scheduler_t;

  /**
   * @brief Inter-task message
   */
  typedef struct
  {
    uint8_t src_task;
    uint8_t dst_task;
    uint8_t msg_type;
    uint8_t flags;
    v4_i32 data;
  } v4_message_t;

  /**
   * @brief Message queue (ring buffer)
   */
  typedef struct
  {
    v4_message_t queue[V4_MSG_QUEUE_SIZE];
    uint8_t read_idx;
    uint8_t write_idx;
    uint8_t count;
    uint8_t reserved;
  } v4_msg_queue_t;

#ifdef __cplusplus
} /* extern "C" */
#endif
