#include "core/addr-mgr.h"

#include <arch.h>

#include "core/macros.h"

typedef struct {
  Tree addr_node;
  Tree size_node;

  uintptr_t begin;
  uintptr_t end;
} Region;

static size_t region_size(const Region* region) {
  assert(region->end >= region->begin);
  return region->end - region->begin;
}

static int TreeVaCmp(Tree* lt, Tree* rt) {
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

static int TreeSizeCmp(Tree* lt, Tree* rt) {
  Region* l = CONTAINER_OF(lt, Region, size_node);
  Region* r = CONTAINER_OF(rt, Region, size_node);

  uintptr_t l_size = region_size(l);
  uintptr_t r_size = region_size(r);

  assert(l_size >= PAGE_SIZE);
  assert(r_size >= PAGE_SIZE);

  if (l_size < r_size) {
    return -1;
  }

  if (l_size > r_size) {
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

void addr_mgr_ctor(AddrMgr* vam) {
  vam->free_by_size = NULL;
  vam->free_by_addr = NULL;
}

static void addr_mgr_free_tree(Tree* t) {
  if (t == NULL) {
    return;
  }
  addr_mgr_free_tree(t->left);
  addr_mgr_free_tree(t->right);

  Region* region = CONTAINER_OF(t, Region, addr_node);
  free(region);
}

void addr_mgr_dtor(AddrMgr* vam) {
  // Both trees point to the same nodes.
  addr_mgr_free_tree(vam->free_by_addr);

  vam->free_by_size = NULL;
  vam->free_by_addr = NULL;
}

static void addr_mgr_insert_region(AddrMgr* vam, Region* region) {
  Tree* existing =
      tree_insert(&vam->free_by_addr, &region->addr_node, &TreeVaCmp);
  assert(existing == NULL);

  existing = tree_insert(&vam->free_by_size, &region->size_node, TreeSizeCmp);
  assert(existing == NULL);
}

static void addr_mgr_erase_region(AddrMgr* vam, Region* region) {
  Tree* deleted =
      tree_erase(&vam->free_by_size, &region->size_node, TreeSizeCmp);
  assert(deleted == &region->size_node);

  deleted = tree_erase(&vam->free_by_addr, &region->addr_node, TreeVaCmp);
  assert(deleted == &region->addr_node);
}

int addr_mgr_add_vas(AddrMgr* vam, uintptr_t va, size_t num_pages) {
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

  addr_mgr_insert_region(vam, region);
  return 0;
}

static Region* addr_mgr_alloc_find_region(const Tree* t, size_t size) {
  if (t == NULL) {
    return NULL;
  }

  Region* r = CONTAINER_OF(t, Region, size_node);
  size_t r_size = region_size(r);

  // Exact size.
  if (r_size == size) {
    return r;
  }

  // Too small, check larger regions.
  if (r_size < size) {
    return addr_mgr_alloc_find_region(t->right, size);
  }

  // Bigger than needed, so try to find a smaller one.
  Region* l_region = addr_mgr_alloc_find_region(t->left, size);
  if (l_region != NULL) {
    return l_region;
  }

  return r;
}

uintptr_t addr_mgr_alloc(AddrMgr* vam, size_t num_pages) {
  size_t size = num_pages * PAGE_SIZE;

  Region* region = addr_mgr_alloc_find_region(vam->free_by_size, size);
  if (region == NULL) {
    return 0;
  }

  const uintptr_t ret = region->begin;
  addr_mgr_erase_region(vam, region);

  // Update the region.
  region->begin += size;

  // Allocated whole region.
  if (region_size(region) == 0) {
    free(region);
    return ret;
  }

  // Store second half in free lists.
  addr_mgr_insert_region(vam, region);

  return ret;
}

static Region* find_adjacent_left(Tree* t, uintptr_t begin) {
  if (t == NULL) {
    return NULL;
  }

  Region* region = CONTAINER_OF(t, Region, addr_node);
  if (begin < region->end) {
    return find_adjacent_left(t->left, begin);
  }

  if (begin > region->end) {
    return find_adjacent_left(t->right, begin);
  }

  return region;
}

static Region* find_adjacent_right(Tree* t, uintptr_t end) {
  if (t == NULL) {
    return NULL;
  }

  Region* region = CONTAINER_OF(t, Region, addr_node);
  if (end < region->begin) {
    return find_adjacent_right(t->left, end);
  }

  if (end > region->begin) {
    return find_adjacent_right(t->right, end);
  }

  return region;
}

void addr_mgr_free(AddrMgr* vam, uintptr_t addr, size_t num_pages) {
  if (num_pages == 0) {
    return;
  }

  size_t size = num_pages * PAGE_SIZE;
  Region key = {
      .begin = addr,
      .end = addr + size,
  };
  if (tree_find(vam->free_by_addr, &key.addr_node, TreeVaCmp) != NULL) {
    PANIC("%s: Double free of VA: [%p, %p)", __func__, (void*)key.begin,
          (void*)key.end);
  }

  uintptr_t new_begin = key.begin;
  uintptr_t new_end = key.end;
  Region* new_region = NULL;

  // See if we can coalesce adjacent regions.
  Region* adjacent_left = find_adjacent_left(vam->free_by_addr, key.begin);
  if (adjacent_left) {
    new_begin = adjacent_left->begin;
    addr_mgr_erase_region(vam, adjacent_left);

    new_region = adjacent_left;
  }

  Region* adjacent_right = find_adjacent_right(vam->free_by_addr, key.end);
  if (adjacent_right) {
    new_end = adjacent_right->end;
    addr_mgr_erase_region(vam, adjacent_right);

    if (new_region != NULL) {
      free(adjacent_right);
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
  addr_mgr_insert_region(vam, new_region);
}
