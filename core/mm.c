#include "core/mm.h"

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/addr-mgr.h"
#include "core/macros.h"
#include "libc/macros.h"
#include "libc/malloc.h"

static AddrMgr g_kernel_va_mgr;
static AddrMgr g_pa_mgr;

// Page allocation requires requires heap allocation.
// Heap allocation requires page allocation.
//
// To solve chicken/egg problem use a single static page.
typedef struct {
  char data[PAGE_SIZE];
} Page;

_Alignas(sizeof(Page)) Page g_default_page;
bool g_default_page_used = false;

void* __malloc_alloc_pages(size_t count) {
  if (count <= 0) {
    return NULL;
  }

  if (!g_default_page_used) {
    assert(count == 1);
    g_default_page_used = true;
    return &g_default_page;
  }

  // TODO(bcf): Implement.
  PANIC("%s: Unimplemented", __func__);
  return NULL;
}

void __malloc_free_page(void* addr) {
  if (addr == &g_default_page) {
    g_default_page_used = false;
    return;
  }

  // TODO(bcf): Implement.
  PANIC("%s: Unimplemented", __func__);
}

void mm_init(void) {
  __malloc_init();

  addr_mgr_ctor(&g_kernel_va_mgr);

  uintptr_t num_heap_pages = (0 - PAGE_SIZE - KERNEL_HEAP_VA) / PAGE_SIZE;
  int err = addr_mgr_add_vas(&g_kernel_va_mgr, KERNEL_HEAP_VA, num_heap_pages);
  PANIC_IF(err != 0, "Registering virtual addresses failed");

  PhysAddrRange paddr_ranges[32];
  int num_paddr_ranges = ARRAY_SIZE(paddr_ranges);
  mm_arch_get_paddr_ranges(paddr_ranges, &num_paddr_ranges);

  for (int i = 0; i < num_paddr_ranges; ++i) {
    PhysAddrRange* range = &paddr_ranges[i];
    size_t num_pages = (range->end - range->begin) / PAGE_SIZE;

    err = addr_mgr_add_vas(&g_pa_mgr, range->begin, num_pages);
    PANIC_IF(err != 0, "Registering physical addresses failed");
  }
}

VirtAddr mm_alloc_page_va(size_t num_pages) {
  return addr_mgr_alloc(&g_kernel_va_mgr, num_pages);
}

void mm_free_page_va(VirtAddr addr, size_t num_pages) {
  addr_mgr_free(&g_kernel_va_mgr, addr, num_pages);
}

PhysAddr mm_alloc_page_pa(void) { return addr_mgr_alloc(&g_pa_mgr, 1); }

void mm_free_page_pa(PhysAddr addr) { addr_mgr_free(&g_pa_mgr, addr, 1); }
