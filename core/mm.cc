#include "core/mm.h"

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <algorithm>

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

alignas(sizeof(Page)) Page g_default_page;
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

static void mm_register_pa(uintptr_t begin, uintptr_t end) {
  if (begin == end) {
    return;
  }

  printf("Registering PAs: [%x, %x)\n", begin, end);
  int err = addr_mgr_add_vas(&g_pa_mgr, begin, (end - begin) / PAGE_SIZE);
  PANIC_IF(err != 0, "Registering physical addresses failed");
}

void mm_init(multiboot_info_t* mbd) {
  if (!((mbd->flags >> 6) & 1)) {
    PANIC("invalid memory map given by GRUB bootloader");
  }

  __malloc_init();

  addr_mgr_ctor(&g_kernel_va_mgr);

  uintptr_t num_heap_pages = (0 - PAGE_SIZE - KERNEL_HEAP_VA) / PAGE_SIZE;
  int err = addr_mgr_add_vas(&g_kernel_va_mgr, KERNEL_HEAP_VA, num_heap_pages);
  PANIC_IF(err != 0, "Registering virtual addresses failed");

  const uintptr_t kernel_begin = mm_arch_kernel_start();
  const uintptr_t kernel_end = mm_arch_kernel_end();
  printf("kernel_pa: [%x, %x)\n", kernel_begin, kernel_end);

  for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
    auto* mmmt = reinterpret_cast<multiboot_memory_map_t*>(mbd->mmap_addr + i);
    if (mmmt->type != MULTIBOOT_MEMORY_AVAILABLE) {
      continue;
    }

    uintptr_t begin = mmmt->addr;
    uintptr_t end = begin + mmmt->len;

    if (kernel_begin >= begin && kernel_end <= end) {
      mm_register_pa(begin, kernel_begin);

      begin = kernel_end;
      end = std::max(begin, end);
    }

    if (kernel_end >= begin && kernel_end <= end) {
      begin = kernel_end;
    }

    mm_register_pa(begin, end);
  }

  // TODO(bcf): Unmap identify page table mappings.
}

VirtAddr mm_alloc_page_va(size_t num_pages) {
  return addr_mgr_alloc(&g_kernel_va_mgr, num_pages);
}

void mm_free_page_va(VirtAddr addr, size_t num_pages) {
  addr_mgr_free(&g_kernel_va_mgr, addr, num_pages);
}

PhysAddr mm_alloc_page_pa(void) { return addr_mgr_alloc(&g_pa_mgr, 1); }

void mm_free_page_pa(PhysAddr addr) { addr_mgr_free(&g_pa_mgr, addr, 1); }
