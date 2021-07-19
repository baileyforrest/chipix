#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "core/ref-cnt.h"
#include "core/types.h"
#include "third_party/multiboot.h"

struct Pages;

namespace mm {
void FreePages(Pages* pages);
}  // namespace mm

// Contiguous physical pages.
struct Pages {
  VirtAddr va{0};
  PhysAddr pa{0};
  int count = 0;

  int RefCnt() const { return ref_cnt; }
  int IncRef() { return ++ref_cnt; }
  int DecRef() {
    assert(ref_cnt > 0);
    return --ref_cnt;
  }

 private:
  // TODO(bcf): Should be atomic.
  int ref_cnt = 1;
};

using PagesRef = Ref<Pages, mm::FreePages>;

namespace mm {

void Init(multiboot_info_t* mbd);

// Returns NULL on failure.
PagesRef AllocPages(size_t count);
void FreePages(Pages* pages);

// Returns kInvalidVa on failure.
VirtAddr AllocPagesVa(size_t num_pages);
void FreePagesVa(VirtAddr addr, size_t num_pages);

// Returns kInvalidPa on failure.
PhysAddr AllocPagesPa(size_t num_pages);
void FreePagesPa(PhysAddr addr, size_t num_pages);

}  // namespace mm

namespace arch {

struct PageTableRoot;
extern PageTableRoot* cur_page_table;

uintptr_t KernelBegin();
uintptr_t KernelEnd();

void SetPageTable(PageTableRoot* page_table);
void FlushTlb();

int MapAddr(PageTableRoot* page_table, VirtAddr va, PhysAddr pa,
            size_t num_pages);
void UnmapAddr(PageTableRoot* page_table, VirtAddr va, size_t num_pages);
PhysAddr LookupPa(PageTableRoot* page_table, VirtAddr va);

}  // namespace arch
