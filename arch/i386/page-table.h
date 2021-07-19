#pragma once

#include <assert.h>
#include <stddef.h>

#include "arch/i386/memory.h"
#include "core/types.h"
#include "libc/macros.h"

namespace arch {

enum class PageSize : u32 {
  k4K = 0,
  k4M = 1,
};

struct PageDirectoryEntry {
  union {
    struct {
      bool present : 1;
      bool writable : 1;
      bool user_access : 1;
      bool write_through : 1;
      bool cache_disabled : 1;
      bool accessed : 1;
      u32 reserved2 : 1;
      PageSize page_size : 1;
      u32 reserved1 : 1;
      u32 avail : 3;
      u32 addr : 20;
    };
    u32 bits;
  };
};
static_assert(sizeof(PageDirectoryEntry) == sizeof(u32));

struct alignas(PAGE_SIZE) PageDirectory {
  static constexpr int kSize = PAGE_SIZE / sizeof(PageDirectoryEntry);

  volatile PageDirectoryEntry& operator[](int index) {
    assert(index >= 0);
    assert(index < ARRAY_SIZE(entries));
    return entries[index];
  }

  size_t size() const { return kSize; }

  volatile PageDirectoryEntry entries[kSize];
};
static_assert(sizeof(PageDirectory) == PAGE_SIZE);

struct PageTableEntry {
  union {
    struct {
      bool present : 1;
      bool writable : 1;
      bool user_access : 1;
      bool write_through : 1;
      bool cache_disabled : 1;
      bool accessed : 1;
      bool dirty : 1;
      u32 reserved1 : 1;
      u32 global : 1;
      u32 avail : 3;
      u32 addr : 20;
    };
    u32 bits;
  };
};

struct alignas(PAGE_SIZE) PageTable {
  static constexpr int kSize = PAGE_SIZE / sizeof(PageTableEntry);
  static constexpr int kBytes = kSize * PAGE_SIZE;

  volatile PageTableEntry& operator[](int index) {
    assert(index >= 0);
    assert(index < ARRAY_SIZE(entries));
    return entries[index];
  }

  size_t size() const { return kSize; }

  volatile PageTableEntry entries[kSize];
};
static_assert(sizeof(PageTable) == PAGE_SIZE);

}  // namespace arch
