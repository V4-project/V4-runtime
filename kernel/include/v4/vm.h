#pragma once
#include <v4/opcodes.h>
#include <v4/sys_ids.h>

#include "v4/task.h"
#include "v4/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @file vm.h
   * @brief V4 VM Public API
   *
   * Main interface for the V4 virtual machine.
   */

  /* Forward declarations */
  struct v4_vm;
  struct v4_word;

  /**
   * @brief VM configuration
   */
  typedef struct
  {
    v4_u8 *mem;      /* RAM base pointer */
    v4_u32 mem_size; /* RAM size in bytes */
  } v4_vm_config_t;

  /**
   * @brief Create a new VM instance
   * @param cfg Configuration structure
   * @return VM instance or NULL on failure
   */
  struct v4_vm *v4_vm_create(const v4_vm_config_t *cfg);

  /**
   * @brief Destroy a VM instance
   * @param vm VM instance
   */
  void v4_vm_destroy(struct v4_vm *vm);

  /**
   * @brief Reset VM state
   * @param vm VM instance
   */
  void v4_vm_reset(struct v4_vm *vm);

  /**
   * @brief Register a word in the VM dictionary
   * @param vm VM instance
   * @param name Word name (can be NULL)
   * @param code Bytecode pointer
   * @param code_len Bytecode length
   * @return Word index (>= 0) or error code (< 0)
   */
  int v4_vm_register_word(struct v4_vm *vm, const char *name, const v4_u8 *code,
                          int code_len);

  /**
   * @brief Find a word by name
   * @param vm VM instance
   * @param name Word name
   * @return Word index (>= 0) or error code (< 0)
   */
  int v4_vm_find_word(struct v4_vm *vm, const char *name);

  /**
   * @brief Execute a word by index
   * @param vm VM instance
   * @param word_idx Word index
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_exec(struct v4_vm *vm, int word_idx);

  /**
   * @brief Execute raw bytecode
   * @param vm VM instance
   * @param bytecode Bytecode pointer
   * @param len Bytecode length
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_exec_raw(struct v4_vm *vm, const v4_u8 *bytecode, int len);

  /**
   * @brief Get data stack depth
   * @param vm VM instance
   * @return Stack depth
   */
  int v4_vm_ds_depth(struct v4_vm *vm);

  /**
   * @brief Peek at data stack value
   * @param vm VM instance
   * @param index Index from top (0 = TOS)
   * @return Stack value or 0 if out of range
   */
  v4_i32 v4_vm_ds_peek(struct v4_vm *vm, int index);

  /**
   * @brief Push value to data stack
   * @param vm VM instance
   * @param value Value to push
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_ds_push(struct v4_vm *vm, v4_i32 value);

  /**
   * @brief Pop value from data stack
   * @param vm VM instance
   * @param out_value Output pointer (can be NULL)
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_ds_pop(struct v4_vm *vm, v4_i32 *out_value);

  /**
   * @brief Read 32-bit value from memory
   * @param vm VM instance
   * @param addr Address
   * @param out Output pointer
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_mem_read32(struct v4_vm *vm, v4_u32 addr, v4_u32 *out);

  /**
   * @brief Write 32-bit value to memory
   * @param vm VM instance
   * @param addr Address
   * @param val Value
   * @return 0 on success, negative on error
   */
  v4_err v4_vm_mem_write32(struct v4_vm *vm, v4_u32 addr, v4_u32 val);

  /**
   * @brief Get VM version
   * @return Version number
   */
  int v4_vm_version(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
