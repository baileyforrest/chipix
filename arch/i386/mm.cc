#include "core/mm.h"

#include <arch.h>
#include <stdio.h>

extern "C" const char _kernel_begin;
extern "C" const char _kernel_end;

namespace arch {

PageDirectory* cur_page_dir;

uintptr_t KernelBegin() { return (uintptr_t)&_kernel_begin; }

uintptr_t KernelEnd() {
  return (uintptr_t)&_kernel_end - KERNEL_HIGH_VA;
}

void SetPageDir(PageDirectory* page_dir) {
  // TODO(bcf): Convert virt to phys
  asm ("movl %0, %%cr3;"
      :
      : "r"(page_dir)
      :);
}

void FlushTlb() {
  asm ("movl %%cr3, %%eax;"
      " movl %%eax, %%cr3;"
      :
      :
      :"%eax");
}

}  // namespace arch
