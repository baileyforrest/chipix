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

// Page table allocation requires page tables to be mapped so we can set them.
// We need a page table to be able to map memory.
//
// To solve chicken/egg problem use a single static page table.
//
// TODO(bcf): To permanently fix this, we need to make sure to never run out of
// kernel VA page tables.
//
// TODO(bcf): Malloc needs to do this too, since page allocation requires heap
// allaction, we need to make sure heap never completely runs out.
alignas(PAGE_SIZE) PageTable g_heap_pt0;
Pages g_heap_pt0_pages;

void Init() {
  {
    // Make sure we never try to deallocate this "Page".
    g_boot_pd_pages.IncRef();
    auto boot_pd_va = reinterpret_cast<uintptr_t>(&__boot_page_directory);
    g_boot_pd_pages.va = VirtAddr(boot_pd_va);
    g_boot_pd_pages.pa = PhysAddr(boot_pd_va - KERNEL_HIGH_VA);
    g_boot_pd_pages.count = 1;
  }
  {
    // Make sure we never try to deallocate this "Page".
    g_heap_pt0_pages.IncRef();
    auto heap_pt0_va = reinterpret_cast<uintptr_t>(&g_heap_pt0);
    g_heap_pt0_pages.va = VirtAddr(heap_pt0_va);
    g_heap_pt0_pages.pa = PhysAddr(heap_pt0_va - KERNEL_HIGH_VA);
    g_heap_pt0_pages.count = 1;

    for (auto& item : g_heap_pt0.entries) {
      item.bits = 0;
    }

    int pde_idx = KERNEL_HEAP_VA / PageTable::kBytes;
    g_boot_pt_root.page_table_pages()[pde_idx] = PagesRef(&g_heap_pt0_pages);
    g_boot_pt_root.SetPde(pde_idx, g_heap_pt0_pages.pa);
  }

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
