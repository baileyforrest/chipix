#pragma once

#include <stddef.h>

#include "core/mm.h"
#include "core/tree.h"

typedef struct {
  Tree* va_by_size;
  Tree* va_by_addr;
} VaMgr;

void va_mgr_ctor(VaMgr* vam);
void va_mgr_dtor(VaMgr* vam);

int va_mgr_add_vas(VaMgr* vam, VirtAddr va, size_t num_pages);

VirtAddr va_mgr_alloc(VaMgr* vam, size_t num_pages);
void va_mgr_free(VaMgr* vam, VirtAddr addr, size_t num_pages);
