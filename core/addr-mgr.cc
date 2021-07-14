#include "core/addr-mgr.h"

#include <arch.h>

#include "core/macros.h"

struct AddrMgr::Region {
  size_t size() const {
    assert(end >= begin);
    return end - begin;
  }

  Tree addr_node;
  Tree size_node;

  uintptr_t begin = 0;
  uintptr_t end = 0;
};

namespace {

using Region = AddrMgr::Region;

int TreeVaCmp(Tree* lt, Tree* rt) {
  Region* l = CONTAINER_OF(lt, Region, addr_node);
  Region* r = CONTAINER_OF(rt, Region, addr_node);

  if (l->end <= r->begin) {
    return -1;
  }

  if (r->end <= l->begin) {
    return 1;
  }

  return 0;
}

int TreeSizeCmp(Tree* lt, Tree* rt) {
  Region* l = CONTAINER_OF(lt, Region, size_node);
  Region* r = CONTAINER_OF(rt, Region, size_node);

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

void FreeTree(Tree* t) {
  if (t == nullptr) {
    return;
  }
  FreeTree(t->left);
  FreeTree(t->right);

  Region* region = CONTAINER_OF(t, Region, addr_node);
  delete region;
}

Region* FindRegion(const Tree* t, size_t size) {
  if (t == NULL) {
    return NULL;
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
  if (l_region != NULL) {
    return l_region;
  }

  return r;
}

Region* FindAdjacentLeft(Tree* t, uintptr_t begin) {
  if (t == NULL) {
    return NULL;
  }

  Region* region = CONTAINER_OF(t, Region, addr_node);
  if (begin < region->end) {
    return FindAdjacentLeft(t->left, begin);
  }

  if (begin > region->end) {
    return FindAdjacentLeft(t->right, begin);
  }

  return region;
}

Region* FindAdjacentRight(Tree* t, uintptr_t end) {
  if (t == NULL) {
    return NULL;
  }

  Region* region = CONTAINER_OF(t, Region, addr_node);
  if (end < region->begin) {
    return FindAdjacentRight(t->left, end);
  }

  if (end > region->begin) {
    return FindAdjacentRight(t->right, end);
  }

  return region;
}

}  // namespace

AddrMgr::~AddrMgr() {
  // Both trees point to the same nodes.
  FreeTree(free_by_addr_);
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
  if (region == NULL) {
    return -1;
  }
  region->begin = begin;
  region->end = end;

  InsertRegion(*region);
  return 0;
}

uintptr_t AddrMgr::Alloc(size_t num_pages) {
  size_t size = num_pages * PAGE_SIZE;
  Region* region = FindRegion(free_by_size_, size);
  if (region == NULL) {
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

  size_t size = num_pages * PAGE_SIZE;
  Region key = {
      .begin = addr,
      .end = addr + size,
  };
  if (tree_find(free_by_addr_, &key.addr_node, TreeVaCmp) != NULL) {
    PANIC("%s: Double free of VA: [%p, %p)", __func__, (void*)key.begin,
          (void*)key.end);
  }

  uintptr_t new_begin = key.begin;
  uintptr_t new_end = key.end;
  Region* new_region = NULL;

  // See if we can coalesce adjacent regions.
  Region* adjacent_left = FindAdjacentLeft(free_by_addr_, key.begin);
  if (adjacent_left) {
    new_begin = adjacent_left->begin;
    EraseRegion(*adjacent_left);

    new_region = adjacent_left;
  }

  Region* adjacent_right = FindAdjacentRight(free_by_addr_, key.end);
  if (adjacent_right) {
    new_end = adjacent_right->end;
    EraseRegion(*adjacent_right);

    if (new_region != NULL) {
      delete adjacent_right;
    } else {
      new_region = adjacent_right;
    }
  }

  if (new_region == NULL) {
    new_region = new Region;
    if (new_region == NULL) {
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
  Tree* existing = tree_insert(&free_by_addr_, &region.addr_node, &TreeVaCmp);
  assert(existing == NULL);

  existing = tree_insert(&free_by_size_, &region.size_node, TreeSizeCmp);
  assert(existing == NULL);
}

void AddrMgr::EraseRegion(Region& region) {
  Tree* deleted = tree_erase(&free_by_size_, &region.size_node, TreeSizeCmp);
  assert(deleted == &region.size_node);

  deleted = tree_erase(&free_by_addr_, &region.addr_node, TreeVaCmp);
  assert(deleted == &region.addr_node);
}

