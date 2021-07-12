#include "core/mm.h"

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/macros.h"
#include "core/va-mgr.h"
#include "libc/macros.h"
#include "libc/malloc.h"

extern const uintptr_t _kernel_start;
extern const uintptr_t _kernel_end;

static VaMgr g_kernel_va_mgr;

static PhysAddrRange g_paddr_ranges[16];
static int g_num_paddr_ranges;

// Page allocation requires requires heap allocation.
// Heap allocation requires page allocation.
//
// To solve chicken/egg problem use a single static page.
typedef struct {
  char data[PAGE_SIZE];
} Page;

_Alignas(sizeof(Page)) Page g_default_page;
bool g_default_page_used = false;

void* __malloc_alloc_pages(int count) {
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

  va_mgr_ctor(&g_kernel_va_mgr);

  uintptr_t heap_size = 0 - PAGE_SIZE - KERNEL_HEAP_VA;
  int err =
      va_mgr_add_vas(&g_kernel_va_mgr, KERNEL_HEAP_VA, heap_size / PAGE_SIZE);
  assert(err == 0);

  g_num_paddr_ranges = ARRAY_SIZE(g_paddr_ranges);
  mm_arch_get_paddr_ranges(g_paddr_ranges, &g_num_paddr_ranges);
}
