#include "core/mm.h"

#include <arch.h>
#include <stdio.h>

#include "arch/i386/page-table-root.h"

extern "C" const char __kernel_begin;
extern "C" const char __kernel_end;

namespace arch {

PageTableRoot* cur_page_table;

uintptr_t KernelBegin() { return reinterpret_cast<uintptr_t>(&__kernel_begin); }

uintptr_t KernelEnd() {
  return reinterpret_cast<uintptr_t>(&__kernel_end) - KERNEL_HIGH_VA;
}

void SetPageTable(PageTableRoot* page_table) {
  asm("movl %0, %%cr3;" : : "r"(page_table->directory_pa().val()) :);
  cur_page_table = page_table;
}

void FlushTlb() {
  asm("movl %%cr3, %%eax;"
      "movl %%eax, %%cr3;"
      :
      :
      : "%eax");
}

int MapAddr(PageTableRoot* page_table, VirtAddr va, PhysAddr pa,
            size_t num_pages) {
  return page_table->MapAddr(va, pa, num_pages);
}

void UnmapAddr(PageTableRoot* page_table, VirtAddr va, size_t num_pages) {
  page_table->UnmapAddr(va, num_pages);
}

PhysAddr LookupPa(PageTableRoot* page_table, VirtAddr va) {
  return page_table->LookupPa(va);
}

}  // namespace arch
