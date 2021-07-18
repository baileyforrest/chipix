#include "arch/i386/include/arch.h"

#include "arch/i386/page-table.h"
#include "core/mm.h"

namespace arch {

extern "C" const char __text_begin;
extern "C" const char __text_end;

extern "C" const char __rodata_begin;
extern "C" const char __rodata_end;

extern "C" PageDirectory __boot_page_directory;
extern "C" PageTable __boot_page_table1;


void Init() {
  cur_page_dir = &__boot_page_directory;

  // Clear identity mappings.
  __boot_page_directory[0].Clear();

  // Clear mappings from before `KernelBegin()`.
  assert(KernelBegin() % PAGE_SIZE == 0);
  int kernel_begin_idx = KernelBegin() / PAGE_SIZE;
  assert(kernel_begin_idx < PageTable::kSize);

  for (int i = 0; i < kernel_begin_idx; ++i) {
    __boot_page_table1[i].Clear();
  }

  // Make text read only.
  for (auto va = reinterpret_cast<uintptr_t>(&__text_begin);
      va <= reinterpret_cast<uintptr_t>(&__text_end); va += PAGE_SIZE) {
    int pt_idx = (va - KERNEL_HIGH_VA) / PAGE_SIZE;
    __boot_page_table1[pt_idx].writable = false;
  }

  // Make rodata read only.
  for (auto va = reinterpret_cast<uintptr_t>(&__rodata_begin);
      va <= reinterpret_cast<uintptr_t>(&__rodata_end); va += PAGE_SIZE) {
    int pt_idx = (va - KERNEL_HIGH_VA) / PAGE_SIZE;
    __boot_page_table1[pt_idx].writable = false;
  }

  FlushTlb();
}

}  // namespace arch
