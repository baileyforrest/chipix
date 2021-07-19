#include "core/addr-mgr.h"

#include <arch.h>

#include "core/macros.h"

using Region = AddrMgr::Region;

struct AddrMgr::Region {
  size_t size() const {
    assert(end >= begin);
    return end - begin;
  }

  AvlNode addr_node;
  AvlNode size_node;

  uintptr_t begin = 0;
  uintptr_t end = 0;
};

int AddrMgr::CompareVa(AvlNode* lhs, AvlNode* rhs) {
  Region* l = CONTAINER_OF(lhs, Region, addr_node);
  Region* r = CONTAINER_OF(rhs, Region, addr_node);

  if (l->end <= r->begin) {
    return -1;
  }

  if (r->end <= l->begin) {
    return 1;
  }

  return 0;
}

int AddrMgr::CompareSize(AvlNode* lhs, AvlNode* rhs) {
  Region* l = CONTAINER_OF(lhs, Region, size_node);
  Region* r = CONTAINER_OF(rhs, Region, size_node);

  assert(l->size() >= PAGE_SIZE);
  assert(r->size() >= PAGE_SIZE);

  if (l->size() < r->size()) {
    return -1;
  }

  if (l->size() > r->size()) {
    return 1;
  }

  if (l->end <= r->begin) {
    return -1;
  }

  if (r->end <= l->begin) {
    return 1;
  }

  return 0;
}

namespace {

Region* FindRegion(const AvlNode* t, size_t size) {
  if (t == nullptr) {
    return nullptr;
  }

  Region* r = CONTAINER_OF(t, Region, size_node);

  // Exact size.
  if (r->size() == size) {
    return r;
  }

  // Too small, check larger regions.
  if (r->size() < size) {
    return FindRegion(t->right, size);
  }

  // Bigger than needed, so try to find a smaller one.
  Region* l_region = FindRegion(t->left, size);
  if (l_region != nullptr) {
    return l_region;
  }

  return r;
}

}  // namespace

AddrMgr::~AddrMgr() {
  // Both trees point to the same nodes.
  free_by_addr_.ForEachBottomUp([](AvlNode* node) {
    Region* region = CONTAINER_OF(node, Region, addr_node);
    delete region;
  });
}

int AddrMgr::AddVas(uintptr_t va, size_t num_pages) {
  if (num_pages == 0) {
    return 0;
  }

  uintptr_t begin = va;
  uintptr_t end = begin + num_pages * PAGE_SIZE;
  if (end < begin) {
    LOG("%s: VA overflow: va: %p, num_pages: %d", __func__, (void*)va,
        (int)num_pages);
    return -1;
  }

  Region* region = new Region;
  if (region == nullptr) {
    return -1;
  }
  region->begin = begin;
  region->end = end;

  InsertRegion(*region);
  return 0;
}

uintptr_t AddrMgr::Alloc(size_t num_pages) {
  size_t size = num_pages * PAGE_SIZE;
  Region* region = FindRegion(free_by_size_.root(), size);
  if (region == nullptr) {
    return 0;
  }

  const uintptr_t ret = region->begin;
  EraseRegion(*region);

  // Update the region.
  region->begin += size;

  // Allocated whole region.
  if (region->size() == 0) {
    delete region;
    return ret;
  }

  // Store second half in free lists.
  InsertRegion(*region);

  return ret;
}

void AddrMgr::Free(uintptr_t addr, size_t num_pages) {
  if (num_pages == 0) {
    return;
  }

  const uintptr_t begin = addr;
  const uintptr_t end = num_pages * PAGE_SIZE;

  {
    AvlNode* existing = free_by_addr_.Find([&](AvlNode* node) {
      Region* region = CONTAINER_OF(node, Region, addr_node);
      if (end < region->begin) {
        return -1;
      }

      if (begin > region->end) {
        return 1;
      }

      return 0;
    });
    if (existing != nullptr) {
      PANIC("%s: Double free of VA: [%p, %p)", __func__, (void*)begin,
            (void*)end);
    }
  }

  uintptr_t new_begin = begin;
  uintptr_t new_end = end;
  Region* new_region = nullptr;

  // See if we can coalesce adjacent regions.
  AvlNode* adjacent_left_node = free_by_addr_.Find([&](AvlNode* node) {
    Region* region = CONTAINER_OF(node, Region, addr_node);
    if (begin < region->end) {
      return -1;
    }

    if (begin > region->end) {
      return 1;
    }

    return 0;
  });

  Region* adjacent_left = nullptr;
  if (adjacent_left_node) {
    adjacent_left = CONTAINER_OF(adjacent_left_node, Region, addr_node);
    new_begin = adjacent_left->begin;
    EraseRegion(*adjacent_left);

    new_region = adjacent_left;
  }

  AvlNode* adjacent_right_node = free_by_addr_.Find([&](AvlNode* node) {
    Region* region = CONTAINER_OF(node, Region, addr_node);
    if (end < region->begin) {
      return -1;
    }

    if (end > region->begin) {
      return 1;
    }

    return 0;
  });

  Region* adjacent_right = nullptr;
  if (adjacent_right_node) {
    new_end = adjacent_right->end;
    EraseRegion(*adjacent_right);

    if (new_region != nullptr) {
      delete adjacent_right;
    } else {
      new_region = adjacent_right;
    }
  }

  if (new_region == nullptr) {
    new_region = new Region;
    if (new_region == nullptr) {
      // TODO(bcf): Handle this robustly.
      LOG("%s: Failed to allocate new free region");
      return;
    }
  }

  new_region->begin = new_begin;
  new_region->end = new_end;
  InsertRegion(*new_region);
}

void AddrMgr::InsertRegion(Region& region) {
  AvlNode* existing = free_by_addr_.Insert(region.addr_node);
  assert(existing == nullptr);

  existing = free_by_size_.Insert(region.size_node);
  assert(existing == nullptr);
}

void AddrMgr::EraseRegion(Region& region) {
  AvlNode* deleted = free_by_size_.Erase(region.size_node);
  assert(deleted == &region.size_node);

  deleted = free_by_addr_.Erase(region.addr_node);
  assert(deleted == &region.addr_node);
}
