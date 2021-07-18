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
uintptr_t KernelBegin();
uintptr_t KernelEnd();
}  // namespace arch
