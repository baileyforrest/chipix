#include "core/mm.h"

#include <arch.h>
#include <stdio.h>

extern "C" char _kernel_begin;
extern "C" char _kernel_end;

namespace arch {

uintptr_t KernelBegin() { return (uintptr_t)&_kernel_begin; }

uintptr_t KernelEnd() {
  return (uintptr_t)&_kernel_end - KERNEL_HIGH_VA;
}

}
