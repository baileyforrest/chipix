#pragma once

#include <assert.h>
#include <stddef.h>

#include "core/intrusive-atl-tree.h"
#include "core/mm.h"

class AddrMgr {
 public:
  AddrMgr() = default;
  ~AddrMgr();

  AddrMgr(const AddrMgr&) = delete;
  AddrMgr operator=(const AddrMgr&) = delete;

  // TODO(bcf): Introduce status type.
  int AddVas(uintptr_t va, size_t num_pages);
  uintptr_t Alloc(size_t num_pages);
  void Free(uintptr_t addr, size_t num_pages);

  struct Region;

 private:
  static int CompareVa(AvlNode* lhs, AvlNode* rhs);
  static int CompareSize(AvlNode* lhs, AvlNode* rhs);

  void InsertRegion(Region& region);
  void EraseRegion(Region& region);

  IntrusiveAvlTree<CompareSize> free_by_size_;
  IntrusiveAvlTree<CompareVa> free_by_addr_;
};
