#pragma once

#include <stddef.h>

#include "core/mm.h"
#include "core/tree.h"

typedef struct {
  Tree* free_by_size;
  Tree* free_by_addr;
} AddrMgr;

void addr_mgr_ctor(AddrMgr* vam);
void addr_mgr_dtor(AddrMgr* vam);

int addr_mgr_add_vas(AddrMgr* vam, uintptr_t va, size_t num_pages);

uintptr_t addr_mgr_alloc(AddrMgr* vam, size_t num_pages);
void addr_mgr_free(AddrMgr* vam, uintptr_t addr, size_t num_pages);
