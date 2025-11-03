#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /* Basic VM types */
  typedef int32_t v4_i32;
  typedef uint32_t v4_u32;
  typedef uint8_t v4_u8;
  typedef int v4_err;

/* Boolean constants (Forth-style) */
#define V4_TRUE ((v4_i32)(-1))
#define V4_FALSE ((v4_i32)(0))

/* Error codes */
#define V4_OK 0
#define V4_ERR_STACK_OVERFLOW (-1)
#define V4_ERR_STACK_UNDERFLOW (-2)
#define V4_ERR_INVALID_OPCODE (-3)
#define V4_ERR_DIV_BY_ZERO (-4)
#define V4_ERR_OUT_OF_BOUNDS (-5)
#define V4_ERR_UNALIGNED (-6)
#define V4_ERR_INVALID_ARG (-7)
#define V4_ERR_NO_MEMORY (-8)
#define V4_ERR_TASK_LIMIT (-9)
#define V4_ERR_MSG_QUEUE_FULL (-10)
#define V4_ERR_NO_MESSAGE (-11)

#ifdef __cplusplus
} /* extern "C" */
#endif
