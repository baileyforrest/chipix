#include "core/mm.h"

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/macros.h"
#include "libc/malloc.h"

extern const uintptr_t _kernel_start;
extern const uintptr_t _kernel_end;

static PhysAddrRange g_paddr_ranges[16];
static int g_num_paddr_ranges;

#if 0
// Bitmap for kernel VAs being available.
// 0 = free
// 1 = used
static char g_kernel_va_map[(0 - KERNEL_HIGH_VA) / sizeof(char)]
static int g_va_next_idx;

// TODO(bcf): Make this dynamic based on memory actually available.
static char g_pa_map[131072];
static int g_pa_map_size;
static int g_pa_next_idx;

static uintptr_t alloc_page_addr(char* map, size_t map_size, int* next_idx) {
  uintptr_t idx = *next_idx
  do {
    char val = map[idx];
    if (val == ~0) {
      if (++idx == map_size) {
        idx = 0;
      }
      continue;
    }

    int bit = 0;
    while (val & (1 << bit)) {
      ++bit;
    }

    *next_idx = idx;

    map[idx] &= (1 << bit);
    uintptr_t page_idx = idx * sizeof(char) + bit;
    return page_idx * PAGE_SIZE;
  } while (idx != g_va_next_idx);

  return 0;
}

static void free_page_addr(char* map, size_t map_size, uintptr_t addr) {
  uintptr_t idx = addr / PAGE_SIZE;
  int bit = idx % sizeof(char);
  idx /= sizeof(char);

  assert(idx < map_size);
  map[idx] |= (1 << bit);
}
#endif

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

#if 0
  // Make kernel VAs as used.
  for (VirtAddr va = _kernel_start; va <= _kernel_end; va += PAGE_SIZE) {
    set_kernel_va_used(va);
  }
#endif

  g_num_paddr_ranges = ARRAY_SIZE(g_paddr_ranges);
  mm_arch_get_paddr_ranges(g_paddr_ranges, &g_num_paddr_ranges);

#if 0
  // TODO(bcf): Set based on number of real entries needed.
  g_pa_map_size = ARRAY_SIZE(g_pa_map);
#endif
}

#if 0
VirtAddr mm_alloc_page_addr_va(int num_pages) {
  assert(num_pages >= 0);
  VirtAddr ret = alloc_page_addr(g_kernel_va_map, ARRAY_SIZE(g_kernel_va_map), &g_va_next_idx);
  if (ret == 0) {
    return 0;
  }
  return ret + KERNEL_HIGH_VA;
}

void mm_free_page_addr_va(VirtAddr addr) {
  assert(addr >= KERNEL_HIGH_VA);

  free_page_addr(g_kernel_va_map, ARRAY_SIZE(g_kernel_va_map), addr - KERNEL_HIGH_VA);
}

PhysAddr mm_alloc_page_addr_pa(void) {
  return alloc_page_addr(g_pa_map, g_pa_map_size, &g_pa_next_idx);
}

void mm_free_page_addr_pa(PhysAddr addr) {
  free_page_addr(g_pa_map, g_pa_map_size, addr);
}
#endif
