#include "arch/i386/page-table-root.h"

#include <new>

namespace arch {

int PageTableRoot::MapAddr(const VirtAddr va, const PhysAddr pa,
                           const size_t num_pages) {
  assert(va.val() % PAGE_SIZE == 0);
  assert(pa.val() % PAGE_SIZE == 0);

  size_t i = 0;

  for (; i < num_pages; ++i) {
    int pde_idx = va.val() / PageTable::kBytes;
    PagesRef& pt_page = page_table_pages_[pde_idx];

    if (!pt_page) {
      pt_page = mm::AllocPages(1);
      if (!pt_page) {
        goto error;
      }

      auto* new_pt = new (reinterpret_cast<void*>(pt_page->va.val())) PageTable;
      for (auto& entry : new_pt->entries) {
        entry.bits = 0;
      }

      PageDirectoryEntry new_pde;
      new_pde.addr = pt_page->pa.val() / PAGE_SIZE;
      new_pde.writable = true;
      new_pde.present = true;

      directory_[pde_idx].bits = new_pde.bits;
    }

    PageTableEntry new_pte;
    new_pte.addr = pa.val() + i * PAGE_SIZE;
    new_pte.writable = true;
    new_pte.present = true;

    auto* page_table = reinterpret_cast<PageTable*>(pt_page->va.val());
    int pte_idx = (va.val() % PageTable::kBytes) / PAGE_SIZE;
    (*page_table)[pte_idx].bits = new_pte.bits;
  }

  return 0;

error:
  UnmapAddr(va, i);
  return -1;
}

void PageTableRoot::UnmapAddr(VirtAddr va, size_t num_pages) {
  assert(va.val() % PAGE_SIZE == 0);
  for (size_t i = 0; i < num_pages; ++i) {
    int pde_idx = va.val() / PageTable::kBytes;
    PagesRef& pt_page = page_table_pages_[pde_idx];
    assert(pt_page);

    auto* page_table = reinterpret_cast<PageTable*>(pt_page->va.val());
    int pte_idx = (va.val() % PageTable::kBytes) / PAGE_SIZE;

    auto& pte = (*page_table)[pte_idx];
    assert(pte.present);
    pte.bits = 0;
  }
}

PhysAddr PageTableRoot::LookupPa(VirtAddr va) {
  int pde_idx = va.val() / PageTable::kBytes;
  PagesRef& pt_page = page_table_pages_[pde_idx];
  if (!pt_page) {
    return PhysAddr(0);
  }

  auto* page_table = reinterpret_cast<PageTable*>(pt_page->va.val());
  int pte_idx = (va.val() % PageTable::kBytes) / PAGE_SIZE;

  auto& pte = (*page_table)[pte_idx];
  if (!pte.present) {
    return PhysAddr(0);
  }

  return PhysAddr(pte.addr * PAGE_SIZE);
}

}  // namespace arch
