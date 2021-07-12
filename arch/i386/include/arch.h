#pragma once

#define PAGE_SIZE 4096
#define KERNEL_HIGH_VA ((uintptr_t)0xc0000000)

// Address where we start allocating dynamic kernel VAs.
#define KERNEL_HEAP_VA ((uintptr_t)0xc0400000)
