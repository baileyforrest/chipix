#include "core/va-mgr.h"

#include <arch.h>

#include "core/macros.h"

typedef struct {
  Tree addr_node;
  Tree size_node;

  VirtAddr begin;
  VirtAddr end;
} VaRegion;

static size_t region_size(const VaRegion* region) {
  assert(region->end >= region->begin);
  return region->end - region->begin;
}

static int TreeVaCmp(Tree* lt, Tree* rt) {
  VaRegion* l = CONTAINER_OF(lt, VaRegion, addr_node);
  VaRegion* r = CONTAINER_OF(rt, VaRegion, addr_node);

  if (l->end <= r->begin) {
    return -1;
  }

  if (r->end >= l->begin) {
    return 1;
  }

  return 0;
}

static int TreeSizeCmp(Tree* lt, Tree* rt) {
  VaRegion* l = CONTAINER_OF(lt, VaRegion, size_node);
  VaRegion* r = CONTAINER_OF(rt, VaRegion, size_node);

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

  if (l->begin < r->begin) {
    return -1;
  }

  if (l->begin > r->begin) {
    return 1;
  }

  return 0;
}

void va_mgr_ctor(VaMgr* vam) {
  vam->va_by_size = NULL;
  vam->va_by_addr = NULL;
}

static void va_mgr_free_tree(Tree* t) {
  if (t == NULL) {
    return;
  }
  va_mgr_free_tree(t->left);
  va_mgr_free_tree(t->right);

  VaRegion* region = CONTAINER_OF(t, VaRegion, addr_node);
  free(region);
}

void va_mgr_dtor(VaMgr* vam) {
  // Both trees point to the same nodes.
  va_mgr_free_tree(vam->va_by_addr);

  vam->va_by_size = NULL;
  vam->va_by_addr = NULL;
}

static void va_mgr_insert_region(VaMgr* vam, VaRegion* region) {
  Tree* existing =
      tree_insert(&vam->va_by_addr, &region->addr_node, &TreeVaCmp);
  assert(existing == NULL);

  existing = tree_insert(&vam->va_by_size, &region->size_node, TreeSizeCmp);
  assert(existing == NULL);
}

static void va_mgr_erase_region(VaMgr* vam, VaRegion* region) {
  Tree* deleted = tree_erase(&vam->va_by_size, &region->size_node, TreeSizeCmp);
  assert(deleted == &region->size_node);

  deleted = tree_erase(&vam->va_by_addr, &region->addr_node, TreeVaCmp);
  assert(deleted == &region->addr_node);
}

int va_mgr_add_vas(VaMgr* vam, VirtAddr va, size_t num_pages) {
  VirtAddr begin = va;
  VirtAddr end = begin + num_pages * PAGE_SIZE;
  if (end < begin) {
    LOG("%s: VA overflow: va: %p, num_pages: %d", __func__, (void*)va,
        (int)num_pages);
    return -1;
  }

  VaRegion* region = malloc(sizeof(*region));
  if (region == NULL) {
    return -1;
  }
  region->begin = begin;
  region->end = end;

  va_mgr_insert_region(vam, region);
  return 0;
}

static VaRegion* va_mgr_alloc_find_region(const Tree* t, size_t size) {
  if (t == NULL) {
    return NULL;
  }

  VaRegion* r = CONTAINER_OF(t, VaRegion, size_node);
  size_t r_size = region_size(r);

  // Exact size.
  if (r_size == size) {
    return r;
  }

  // Too small, check larger regions.
  if (r_size < size) {
    return va_mgr_alloc_find_region(t->right, size);
  }

  // Bigger than needed, so try to find a smaller one.
  VaRegion* l_region = va_mgr_alloc_find_region(t->left, size);
  if (l_region != NULL) {
    return l_region;
  }

  return r;
}

VirtAddr va_mgr_alloc(VaMgr* vam, size_t num_pages) {
  size_t size = num_pages * PAGE_SIZE;

  VaRegion* region = va_mgr_alloc_find_region(vam->va_by_size, size);
  if (region == NULL) {
    return 0;
  }

  const VirtAddr ret = region->begin;
  va_mgr_erase_region(vam, region);

  // Update the region.
  region->begin += size;

  // Allocated whole region.
  if (region_size(region) == 0) {
    free(region);
    return ret;
  }

  // Store second half in free lists.
  va_mgr_insert_region(vam, region);

  return ret;
}

static VaRegion* find_adjacent_left(Tree* t, VirtAddr begin) {
  if (t == NULL) {
    return NULL;
  }

  VaRegion* region = CONTAINER_OF(t, VaRegion, addr_node);
  if (begin < region->end) {
    return find_adjacent_left(t->left, begin);
  }

  if (begin > region->end) {
    return find_adjacent_left(t->right, begin);
  }

  return region;
}

static VaRegion* find_adjacent_right(Tree* t, VirtAddr end) {
  if (t == NULL) {
    return NULL;
  }

  VaRegion* region = CONTAINER_OF(t, VaRegion, addr_node);
  if (end < region->begin) {
    return find_adjacent_right(t->left, end);
  }

  if (end > region->begin) {
    return find_adjacent_right(t->right, end);
  }

  return region;
}

void va_mgr_free(VaMgr* vam, VirtAddr addr, size_t num_pages) {
  size_t size = num_pages * PAGE_SIZE;
  VaRegion key = {
      .begin = addr,
      .end = addr + size,
  };
  if (tree_find(vam->va_by_addr, &key.addr_node, TreeVaCmp) != NULL) {
    PANIC("%s: Double free of VA: [%p, %p)", __func__, (void*)key.begin,
          (void*)key.end);
  }

  VirtAddr new_begin = key.begin;
  VirtAddr new_end = key.end;
  VaRegion* new_region = NULL;

  // See if we can coalesce adjacent regions.
  VaRegion* adjacent_left = find_adjacent_left(vam->va_by_addr, key.begin);
  if (adjacent_left) {
    new_begin = adjacent_left->begin;
    va_mgr_erase_region(vam, adjacent_left);

    new_region = adjacent_left;
  }

  VaRegion* adjacent_right = find_adjacent_right(vam->va_by_addr, key.end);
  if (adjacent_right) {
    new_end = adjacent_right->end;
    va_mgr_erase_region(vam, adjacent_right);

    if (new_region != NULL) {
      free(adjacent_right);
    } else {
      new_region = adjacent_right;
    }
  }

  if (new_region == NULL) {
    new_region = malloc(sizeof(*new_region));
    if (new_region == NULL) {
      // TODO(bcf): Handle this robustly.
      LOG("%s: Failed to allocate new free region");
      return;
    }
  }

  new_region->begin = new_begin;
  new_region->end = new_end;
  va_mgr_insert_region(vam, new_region);
}
