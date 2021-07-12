#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uintptr_t PhysAddr;
typedef uintptr_t VirtAddr;

typedef struct {
  PhysAddr begin;
  PhysAddr end;
} PhysAddrRange;

void mm_init(void);

// Returns NULL on failure.
VirtAddr mm_alloc_page_va(size_t num_pages);
void mm_free_page_va(VirtAddr addr, size_t num_pages);

// Returns NULL on failure.
PhysAddr mm_alloc_page_pa(void);
void mm_free_page_pa(PhysAddr addr);

void mm_arch_get_paddr_ranges(PhysAddrRange* ranges, int* count);
