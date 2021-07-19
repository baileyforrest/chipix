#include "core/mm.h"

#include <arch.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

#include "core/addr-mgr.h"
#include "core/cleanup.h"
#include "core/macros.h"
#include "libc/macros.h"
#include "libc/malloc.h"

namespace mm {
namespace {

AddrMgr g_kernel_va_mgr;
AddrMgr g_pa_mgr;

// Page allocation requires requires heap allocation.
// Heap allocation requires page allocation.
//
// To solve chicken/egg problem use a single static page.
struct DefaultPage {
  char data[PAGE_SIZE];
};

alignas(PAGE_SIZE) DefaultPage g_default_page;
bool g_default_page_used = false;

PhysAddr AllocAndMapPhysPages(const VirtAddr virt_begin, const size_t count) {
  const PhysAddr phys_begin = AllocPagesPa(count);
  if (phys_begin == PhysAddr(0)) {
    return PhysAddr(0);
  }
  auto clean_pa = MakeCleanup([&] { FreePagesPa(phys_begin, count); });

  if (arch::MapAddr(arch::cur_page_table, virt_begin, phys_begin, count) < 0) {
    return PhysAddr(0);
  }

  std::move(clean_pa).Cancel();
  return phys_begin;
}

}  // namespace

void Init(multiboot_info_t* mbd) {
  if (!((mbd->flags >> 6) & 1)) {
    PANIC("invalid memory map given by GRUB bootloader");
  }

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
}

PagesRef AllocPages(const size_t count) {
  if (count <= 0) {
    return {};
  }

  PagesRef ret(new Pages());
  if (!ret) {
    return {};
  }

  ret->count = count;
  ret->va = AllocPagesVa(count);
  if (ret->va == VirtAddr(0)) {
    return {};
  }
  auto clean_va = MakeCleanup([&] { FreePagesVa(ret->va, count); });

  ret->pa = AllocAndMapPhysPages(ret->va, count);
  if (ret->pa == PhysAddr(0)) {
    return {};
  }

  std::move(clean_va).Cancel();
  return ret;
}

void FreePages(Pages* pages) {
  assert(pages->RefCnt() == 0);

  UnmapAddr(arch::cur_page_table, pages->va, pages->count);
  FreePagesPa(pages->pa, pages->count);
  FreePagesVa(pages->va, pages->count);
  delete pages;
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

void FreePagesPa(PhysAddr addr, size_t num_pages) {
  g_pa_mgr.Free(addr.val(), num_pages);
}

}  // namespace mm

void* __malloc_alloc_pages(const size_t count) {
  if (count <= 0) {
    return nullptr;
  }

  if (!mm::g_default_page_used) {
    assert(count == 1);
    mm::g_default_page_used = true;
    return &mm::g_default_page;
  }

  const VirtAddr virt_begin = mm::AllocPagesVa(count);
  if (virt_begin == VirtAddr(0)) {
    return nullptr;
  }

  // Malloc doesn't need contiguous physical pages.
  size_t i = 0;
  for (; i < count; ++i) {
    VirtAddr va = virt_begin + i * PAGE_SIZE;
    PhysAddr pa = mm::AllocAndMapPhysPages(va, 1);
    if (pa == PhysAddr(0)) {
      goto error;
    }
  }

  return reinterpret_cast<void*>(virt_begin.val());

error:
  arch::UnmapAddr(arch::cur_page_table, virt_begin, i);
  for (size_t j = 0; j < i; ++j) {
    VirtAddr va = virt_begin + j * PAGE_SIZE;
    PhysAddr pa = LookupPa(arch::cur_page_table, va);
    mm::FreePagesPa(pa, 1);
  }

  mm::FreePagesVa(virt_begin, count);
  return nullptr;
}

void __malloc_free_page(void* addr, size_t num_pages) {
  if (addr == &mm::g_default_page) {
    mm::g_default_page_used = false;
    return;
  }

  VirtAddr virt_begin(reinterpret_cast<uintptr_t>(addr));
  for (size_t i = 0; i < num_pages; ++i) {
    PhysAddr pa =
        arch::LookupPa(arch::cur_page_table, virt_begin + i * PAGE_SIZE);
    assert(pa != PhysAddr(0));
    mm::FreePagesPa(pa, 1);
  }

  arch::UnmapAddr(arch::cur_page_table, virt_begin, num_pages);
  mm::FreePagesVa(virt_begin, num_pages);
}
