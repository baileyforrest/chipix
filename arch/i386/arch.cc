#include "arch/i386/include/arch.h"

#include "arch/i386/page-table-root.h"
#include "arch/i386/page-table.h"
#include "core/mm.h"

namespace arch {

extern "C" const char __text_begin;
extern "C" const char __text_end;

extern "C" const char __rodata_begin;
extern "C" const char __rodata_end;

extern "C" PageDirectory __boot_page_directory;
extern "C" PageTable __boot_page_table1;

Pages g_boot_pd_pages;
PageTableRoot g_boot_pt_root(&__boot_page_directory,
                             PagesRef{&g_boot_pd_pages});

void Init() {
  // Make sure we never try to deallocate this "Page".
  g_boot_pd_pages.IncRef();

  auto boot_pd_va = reinterpret_cast<uintptr_t>(&__boot_page_directory);
  g_boot_pd_pages.va = VirtAddr(boot_pd_va);
  g_boot_pd_pages.pa = PhysAddr(boot_pd_va - KERNEL_HIGH_VA);

  cur_page_table = &g_boot_pt_root;

  // Clear identity mappings.
  __boot_page_directory[0].bits = 0;

  // Clear mappings from before `KernelBegin()`.
  assert(KernelBegin() % PAGE_SIZE == 0);
  int kernel_begin_idx = KernelBegin() / PAGE_SIZE;
  assert(kernel_begin_idx < PageTable::kSize);

  for (int i = 0; i < kernel_begin_idx; ++i) {
    __boot_page_table1[i].bits = 0;
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
