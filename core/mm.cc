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

namespace {

static AddrMgr g_kernel_va_mgr;
static AddrMgr g_pa_mgr;

// Page allocation requires requires heap allocation.
// Heap allocation requires page allocation.
//
// To solve chicken/egg problem use a single static page.
struct Page {
  char data[PAGE_SIZE];
};

alignas(sizeof(Page)) Page g_default_page;
bool g_default_page_used = false;

}  // namespace

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

namespace mm {

void Init(multiboot_info_t* mbd) {
  if (!((mbd->flags >> 6) & 1)) {
    PANIC("invalid memory map given by GRUB bootloader");
  }

  __malloc_init();

  uintptr_t num_heap_pages = (0 - PAGE_SIZE - KERNEL_HEAP_VA) / PAGE_SIZE;
  int err = g_kernel_va_mgr.AddVas(KERNEL_HEAP_VA, num_heap_pages);
  PANIC_IF(err != 0, "Registering virtual addresses failed");

  const uintptr_t kernel_begin = arch::KernelBegin();
  const uintptr_t kernel_end = arch::KernelEnd();
  printf("kernel_pa: [%x, %x)\n", kernel_begin, kernel_end);

  for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
    auto* mmmt = reinterpret_cast<multiboot_memory_map_t*>(mbd->mmap_addr + i);
    if (mmmt->type != MULTIBOOT_MEMORY_AVAILABLE) {
      continue;
    }

    uintptr_t begin = mmmt->addr;
    uintptr_t end = begin + mmmt->len;


    auto register_pa = [](uintptr_t begin, uintptr_t end) {
      if (begin == end) {
        return;
      }

      printf("Registering PAs: [%x, %x)\n", begin, end);
      int err = g_pa_mgr.AddVas(begin, (end - begin) / PAGE_SIZE);
      PANIC_IF(err != 0, "Registering physical addresses failed");
    };

    if (kernel_begin >= begin && kernel_end <= end) {
      register_pa(begin, kernel_begin);

      begin = kernel_end;
      end = std::max(begin, end);
    }

    if (kernel_end >= begin && kernel_end <= end) {
      begin = kernel_end;
    }

    register_pa(begin, end);
  }

  // TODO(bcf): Unmap identify page table mappings.
}

VirtAddr AllocPagesVa(size_t num_pages) {
  return VirtAddr(g_kernel_va_mgr.Alloc(num_pages));
}

void FreePagesVa(VirtAddr addr, size_t num_pages) {
  g_kernel_va_mgr.Free(addr.val(), num_pages);
}

PhysAddr AllocPagesPa(size_t num_pages) {
  return PhysAddr(g_pa_mgr.Alloc(num_pages));
}

void FreePagePa(PhysAddr addr, size_t num_pages) {
  g_pa_mgr.Free(addr.val(), num_pages);
}

}  // namespace mm
