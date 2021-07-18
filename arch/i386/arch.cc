#include "arch/i386/include/arch.h"

#include "arch/i386/page-table.h"
#include "core/mm.h"

namespace arch {

extern "C" const char _text_begin;
extern "C" const char _text_end;

extern "C" const char _rodata_begin;
extern "C" const char _rodata_end;

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

  FlushTlb();
}

}  // namespace arch
