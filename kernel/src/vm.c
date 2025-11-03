#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "v4/internal/vm_internal.h"

/* Little-endian readers */
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

/* Internal stack helpers */
v4_err v4_ds_push_internal(v4_vm_t *vm, v4_i32 val)
{
  if (vm->sp >= vm->DS + V4_DS_SIZE)
    return V4_ERR_STACK_OVERFLOW;
  *vm->sp++ = val;
  return V4_OK;
}

v4_err v4_ds_pop_internal(v4_vm_t *vm, v4_i32 *out)
{
  if (vm->sp <= vm->DS)
    return V4_ERR_STACK_UNDERFLOW;
  *out = *--vm->sp;
  return V4_OK;
}

v4_err v4_ds_peek_internal(const v4_vm_t *vm, int idx, v4_i32 *out)
{
  if (vm->sp - 1 - idx < vm->DS)
    return V4_ERR_STACK_UNDERFLOW;
  *out = *(vm->sp - 1 - idx);
  return V4_OK;
}

v4_err v4_rs_push_internal(v4_vm_t *vm, v4_i32 val)
{
  if (vm->rp >= vm->RS + V4_RS_SIZE)
    return V4_ERR_STACK_OVERFLOW;
  *vm->rp++ = val;
  return V4_OK;
}

v4_err v4_rs_pop_internal(v4_vm_t *vm, v4_i32 *out)
{
  if (vm->rp <= vm->RS)
    return V4_ERR_STACK_UNDERFLOW;
  *out = *--vm->rp;
  return V4_OK;
}

/* Public API */
int v4_vm_version(void)
{
  return 1;
}

struct v4_vm *v4_vm_create(const v4_vm_config_t *cfg)
{
  if (!cfg)
    return NULL;

  v4_vm_t *vm = (v4_vm_t *)calloc(1, sizeof(v4_vm_t));
  if (!vm)
    return NULL;

  /* Initialize stacks */
  vm->sp = vm->DS;
  vm->rp = vm->RS;
  vm->fp = NULL;

  /* Memory configuration */
  vm->mem = cfg->mem;
  vm->mem_size = cfg->mem_size;

  /* Initialize scheduler and message queue */
  v4_scheduler_init(&vm->scheduler);
  v4_msg_queue_init(&vm->msg_queue);

  vm->word_count = 0;
  vm->last_err = V4_OK;

  return (struct v4_vm *)vm;
}

void v4_vm_destroy(struct v4_vm *vm)
{
  if (!vm)
    return;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  /* Free word names */
  for (int i = 0; i < vm_internal->word_count; i++)
  {
    if (vm_internal->words[i].name)
    {
      free(vm_internal->words[i].name);
    }
  }

  free(vm);
}

void v4_vm_reset(struct v4_vm *vm)
{
  if (!vm)
    return;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;
  vm_internal->sp = vm_internal->DS;
  vm_internal->rp = vm_internal->RS;
  vm_internal->fp = NULL;
  vm_internal->last_err = V4_OK;
}

int v4_vm_register_word(struct v4_vm *vm, const char *name, const v4_u8 *code,
                        int code_len)
{
  if (!vm || !code || code_len <= 0)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  if (vm_internal->word_count >= V4_MAX_WORDS)
    return V4_ERR_NO_MEMORY;

  int idx = vm_internal->word_count;
  v4_word_t *w = &vm_internal->words[idx];

  w->name = name ? strdup(name) : NULL;
  w->code = code;
  w->code_len = code_len;

  vm_internal->word_count++;
  return idx;
}

int v4_vm_find_word(struct v4_vm *vm, const char *name)
{
  if (!vm || !name)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  for (int i = 0; i < vm_internal->word_count; i++)
  {
    if (vm_internal->words[i].name && strcmp(vm_internal->words[i].name, name) == 0)
    {
      return i;
    }
  }

  return V4_ERR_INVALID_ARG;
}

int v4_vm_ds_depth(struct v4_vm *vm)
{
  if (!vm)
    return 0;
  v4_vm_t *vm_internal = (v4_vm_t *)vm;
  return (int)(vm_internal->sp - vm_internal->DS);
}

v4_i32 v4_vm_ds_peek(struct v4_vm *vm, int index)
{
  if (!vm)
    return 0;
  v4_vm_t *vm_internal = (v4_vm_t *)vm;
  v4_i32 val;
  if (v4_ds_peek_internal(vm_internal, index, &val) != V4_OK)
    return 0;
  return val;
}

v4_err v4_vm_ds_push(struct v4_vm *vm, v4_i32 value)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;
  return v4_ds_push_internal((v4_vm_t *)vm, value);
}

v4_err v4_vm_ds_pop(struct v4_vm *vm, v4_i32 *out_value)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;
  return v4_ds_pop_internal((v4_vm_t *)vm, out_value);
}

v4_err v4_vm_mem_read32(struct v4_vm *vm, v4_u32 addr, v4_u32 *out)
{
  if (!vm || !out)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  if (addr & 3)
    return V4_ERR_UNALIGNED;

  if (addr + 4 > vm_internal->mem_size)
    return V4_ERR_OUT_OF_BOUNDS;

  *out = read_i32_le(vm_internal->mem + addr);
  return V4_OK;
}

v4_err v4_vm_mem_write32(struct v4_vm *vm, v4_u32 addr, v4_u32 val)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  if (addr & 3)
    return V4_ERR_UNALIGNED;

  if (addr + 4 > vm_internal->mem_size)
    return V4_ERR_OUT_OF_BOUNDS;

  vm_internal->mem[addr] = (v4_u8)(val & 0xFF);
  vm_internal->mem[addr + 1] = (v4_u8)((val >> 8) & 0xFF);
  vm_internal->mem[addr + 2] = (v4_u8)((val >> 16) & 0xFF);
  vm_internal->mem[addr + 3] = (v4_u8)((val >> 24) & 0xFF);

  return V4_OK;
}

/* Raw bytecode executor - Forward declaration for next part */
v4_err v4_vm_exec_raw(struct v4_vm *vm, const v4_u8 *bytecode, int len);

v4_err v4_vm_exec(struct v4_vm *vm, int word_idx)
{
  if (!vm)
    return V4_ERR_INVALID_ARG;

  v4_vm_t *vm_internal = (v4_vm_t *)vm;

  if (word_idx < 0 || word_idx >= vm_internal->word_count)
    return V4_ERR_INVALID_ARG;

  const v4_word_t *word = &vm_internal->words[word_idx];
  return v4_vm_exec_raw(vm, word->code, word->code_len);
}
