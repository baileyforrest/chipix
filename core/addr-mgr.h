#pragma once

#include <assert.h>
#include <stddef.h>

#include "core/mm.h"
#include "core/tree.h"

class AddrMgr {
 public:
  struct Region;

  AddrMgr() = default;
  ~AddrMgr();

  AddrMgr(const AddrMgr&) = delete;
  AddrMgr operator=(const AddrMgr&) = delete;

  // TODO(bcf): Introduce status type.
  int AddVas(uintptr_t va, size_t num_pages);
  uintptr_t Alloc(size_t num_pages);
  void Free(uintptr_t addr, size_t num_pages);

 private:
  void InsertRegion(Region& region);
  void EraseRegion(Region& region);

  Tree* free_by_size_ = nullptr;
  Tree* free_by_addr_ = nullptr;
};
