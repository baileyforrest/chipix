#include "core/mm.h"

#include <arch.h>
#include <stdio.h>

#include "core/macros.h"

extern char _kernel_start;
extern char _kernel_end;

uintptr_t mm_arch_kernel_start(void) { return (uintptr_t)&_kernel_start; }

uintptr_t mm_arch_kernel_end(void) {
  return (uintptr_t)&_kernel_end - KERNEL_HIGH_VA;
}
