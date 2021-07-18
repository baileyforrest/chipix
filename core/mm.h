#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/types.h"
#include "third_party/multiboot.h"

namespace mm {

void Init(multiboot_info_t* mbd);

// Returns NULL on failure.
VirtAddr AllocPagesVa(size_t num_pages);
void FreePagesVa(VirtAddr addr, size_t num_pages);

// Returns NULL on failure.
PhysAddr AllocPagesPa(size_t num_pages);
void FreePagesPa(PhysAddr addr, size_t num_pages);

}  // namespace mm

namespace arch {

struct PageDirectory;
extern PageDirectory* cur_page_dir;

uintptr_t KernelBegin();
uintptr_t KernelEnd();

void SetPageDir(PageDirectory* page_dir);
void FlushTlb();

}  // namespace arch
