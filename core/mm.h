#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/multiboot.h"

typedef uintptr_t PhysAddr;
typedef uintptr_t VirtAddr;

void mm_init(multiboot_info_t* mbd);

// Returns NULL on failure.
VirtAddr mm_alloc_page_va(size_t num_pages);
void mm_free_page_va(VirtAddr addr, size_t num_pages);

// Returns NULL on failure.
PhysAddr mm_alloc_page_pa(void);
void mm_free_page_pa(PhysAddr addr);

uintptr_t mm_arch_kernel_start(void);
uintptr_t mm_arch_kernel_end(void);
