#include <string.h>

#include "v4/internal/vm_internal.h"

void v4_msg_queue_init(v4_msg_queue_t *q)
{
  if (!q)
    return;

  memset(q, 0, sizeof(v4_msg_queue_t));
  q->read_idx = 0;
  q->write_idx = 0;
  q->count = 0;
}

v4_err v4_msg_send(v4_vm_t *vm, uint8_t dst_task, uint8_t msg_type, v4_i32 data)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_msg_queue_t *q = &vm->msg_queue;

  /* Queue full? */
  if (q->count >= V4_MSG_QUEUE_SIZE)
    return V4_ERR_MSG_QUEUE_FULL;

  /* Get source task ID */
  uint8_t src_task = vm->scheduler.current_task;

  /* Add message to queue */
  v4_message_t *msg = &q->queue[q->write_idx];
  msg->src_task = src_task;
  msg->dst_task = dst_task;
  msg->msg_type = msg_type;
  msg->flags = 0;
  msg->data = data;

  q->write_idx = (q->write_idx + 1) % V4_MSG_QUEUE_SIZE;
  q->count++;

  return V4_OK;
}

v4_err v4_msg_receive(v4_vm_t *vm, uint8_t msg_type, v4_i32 *data, uint8_t *src_task,
                      int blocking, v4_u32 timeout_ms)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_msg_queue_t *q = &vm->msg_queue;
  uint8_t current_task = vm->scheduler.current_task;

  v4_u32 start_time = 0;
  if (blocking)
  {
    start_time = v4_platform_get_tick_ms();
  }

  while (1)
  {
    /* Search for matching message */
    for (uint8_t i = 0; i < q->count; i++)
    {
      uint8_t idx = (q->read_idx + i) % V4_MSG_QUEUE_SIZE;
      v4_message_t *msg = &q->queue[idx];

      /* Check if message matches:
       * - Message type matches
       * - Destination is current task or broadcast (0xFF)
       */
      if (msg->msg_type == msg_type &&
          (msg->dst_task == current_task || msg->dst_task == 0xFF))
      {
        /* Found matching message */
        if (data)
          *data = msg->data;
        if (src_task)
          *src_task = msg->src_task;

        /* Remove message from queue */
        /* Shift remaining messages */
        for (uint8_t j = i; j < q->count - 1; j++)
        {
          uint8_t curr_idx = (q->read_idx + j) % V4_MSG_QUEUE_SIZE;
          uint8_t next_idx = (q->read_idx + j + 1) % V4_MSG_QUEUE_SIZE;
          q->queue[curr_idx] = q->queue[next_idx];
        }
        q->count--;

        return V4_OK;
      }
    }

    /* No matching message found */
    if (!blocking)
    {
      return V4_ERR_NO_MESSAGE;
    }

    /* Check timeout */
    if (timeout_ms > 0)
    {
      v4_u32 current_time = v4_platform_get_tick_ms();
      if (current_time - start_time >= timeout_ms)
      {
        return V4_ERR_NO_MESSAGE;
      }
    }

    /* Yield to other tasks and try again */
    v4_task_yield(vm);
  }
}
