#include <stdint.h>

#include "v4/internal/vm_internal.h"

/* Forward declarations for helpers from vm.c */
extern v4_err v4_ds_push_internal(v4_vm_t *vm, v4_i32 val);
extern v4_err v4_ds_pop_internal(v4_vm_t *vm, v4_i32 *out);
extern v4_err v4_ds_peek_internal(const v4_vm_t *vm, int idx, v4_i32 *out);
extern v4_err v4_rs_push_internal(v4_vm_t *vm, v4_i32 val);
extern v4_err v4_rs_pop_internal(v4_vm_t *vm, v4_i32 *out);

/* Little-endian readers (duplicated for this file) */
static inline v4_i32 read_i32_le(const v4_u8 *p)
{
  return (v4_i32)((v4_u32)p[0] | ((v4_u32)p[1] << 8) | ((v4_u32)p[2] << 16) |
                  ((v4_u32)p[3] << 24));
}

static inline int16_t read_i16_le(const v4_u8 *p)
{
  return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static inline uint16_t read_u16_le(const v4_u8 *p)
{
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

/**
 * @brief Execute raw bytecode
 *
 * This is a minimal implementation of core opcodes.
 * More opcodes will be added in future commits.
 */
v4_err v4_vm_exec_raw(struct v4_vm *vm, const v4_u8 *bytecode, int len)
{
  if (!vm || !bytecode || len <= 0)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;
  const v4_u8 *ip = bytecode;
  const v4_u8 *ip_end = bytecode + len;

  while (ip < ip_end)
  {
    uint8_t op = *ip++;

    switch (op)
    {
      /* ===== Stack operations ===== */
      case V4_OP_LIT:
      {
        if (ip + 4 > ip_end)
          return V4_ERR_INVALID_OPCODE;
        v4_i32 val = read_i32_le(ip);
        ip += 4;
        v4_err e = v4_ds_push_internal(vm_internal, val);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_DUP:
      {
        v4_i32 a;
        v4_err e = v4_ds_peek_internal(vm_internal, 0, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, a);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_DROP:
      {
        v4_i32 dummy;
        v4_err e = v4_ds_pop_internal(vm_internal, &dummy);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_SWAP:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, a);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, b);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_OVER:
      {
        v4_i32 val;
        v4_err e = v4_ds_peek_internal(vm_internal, 1, &val);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, val);
        if (e != V4_OK)
          return e;
        break;
      }

      /* ===== Arithmetic ===== */
      case V4_OP_ADD:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, b + a);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_SUB:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, b - a);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_MUL:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, b * a);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_DIV:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        if (a == 0)
          return V4_ERR_DIV_BY_ZERO;
        e = v4_ds_push_internal(vm_internal, b / a);
        if (e != V4_OK)
          return e;
        break;
      }

      /* ===== Comparison ===== */
      case V4_OP_EQ:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, (b == a) ? V4_TRUE : V4_FALSE);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_LT:
      {
        v4_i32 a, b;
        v4_err e = v4_ds_pop_internal(vm_internal, &a);
        if (e != V4_OK)
          return e;
        e = v4_ds_pop_internal(vm_internal, &b);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, (b < a) ? V4_TRUE : V4_FALSE);
        if (e != V4_OK)
          return e;
        break;
      }

      /* ===== Control flow ===== */
      case V4_OP_JMP:
      {
        if (ip + 2 > ip_end)
          return V4_ERR_INVALID_OPCODE;
        int16_t offset = read_i16_le(ip);
        ip += 2;
        ip += offset;
        if (ip < bytecode || ip > ip_end)
          return V4_ERR_OUT_OF_BOUNDS;
        break;
      }

      case V4_OP_JZ:
      {
        if (ip + 2 > ip_end)
          return V4_ERR_INVALID_OPCODE;
        int16_t offset = read_i16_le(ip);
        ip += 2;
        v4_i32 cond;
        v4_err e = v4_ds_pop_internal(vm_internal, &cond);
        if (e != V4_OK)
          return e;
        if (cond == 0)
        {
          ip += offset;
          if (ip < bytecode || ip > ip_end)
            return V4_ERR_OUT_OF_BOUNDS;
        }
        break;
      }

      case V4_OP_RET:
      {
        /* Return from current word */
        return V4_OK;
      }

      /* ===== Return stack ===== */
      case V4_OP_TOR:
      {
        v4_i32 val;
        v4_err e = v4_ds_pop_internal(vm_internal, &val);
        if (e != V4_OK)
          return e;
        e = v4_rs_push_internal(vm_internal, val);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_FROMR:
      {
        v4_i32 val;
        v4_err e = v4_rs_pop_internal(vm_internal, &val);
        if (e != V4_OK)
          return e;
        e = v4_ds_push_internal(vm_internal, val);
        if (e != V4_OK)
          return e;
        break;
      }

      /* ===== Compact literals ===== */
      case V4_OP_LIT0:
      {
        v4_err e = v4_ds_push_internal(vm_internal, 0);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_LIT1:
      {
        v4_err e = v4_ds_push_internal(vm_internal, 1);
        if (e != V4_OK)
          return e;
        break;
      }

      /* ===== Task management ===== */
      case V4_OP_TASK_YIELD:
      {
        v4_err e = v4_task_yield(vm_internal);
        if (e != V4_OK)
          return e;
        break;
      }

      case V4_OP_TASK_SLEEP:
      {
        v4_i32 ms;
        v4_err e = v4_ds_pop_internal(vm_internal, &ms);
        if (e != V4_OK)
          return e;
        e = v4_task_sleep(vm_internal, (v4_u32)ms);
        if (e != V4_OK)
          return e;
        break;
      }

      /* Unimplemented opcodes */
      default:
        return V4_ERR_INVALID_OPCODE;
    }
  }

  return V4_OK;
}
