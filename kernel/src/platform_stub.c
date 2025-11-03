#include <time.h>

#include "v4/internal/vm_internal.h"

/**
 * @file platform_stub.c
 * @brief Stub implementation of platform functions for host testing
 *
 * These functions should be replaced by actual platform implementations
 * in the BSP layer for embedded targets.
 */

v4_u32 v4_platform_get_tick_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (v4_u32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

v4_u32 v4_platform_get_tick_us(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (v4_u32)(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

void v4_platform_delay_ms(v4_u32 ms)
{
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

void v4_platform_delay_us(v4_u32 us)
{
  struct timespec ts;
  ts.tv_sec = us / 1000000;
  ts.tv_nsec = (us % 1000000) * 1000;
  nanosleep(&ts, NULL);
}
