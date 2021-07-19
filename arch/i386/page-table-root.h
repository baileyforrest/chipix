#pragma once

#include <utility>

#include "arch/i386/page-table.h"
#include "core/mm.h"

namespace arch {

class PageTableRoot {
 public:
  explicit PageTableRoot(PageDirectory* directory, PagesRef directory_page)
      : directory_(*directory), directory_page_(std::move(directory_page)) {}

  int MapAddr(VirtAddr va, PhysAddr pa, size_t num_pages);
  void UnmapAddr(VirtAddr va, size_t num_pages);
  PhysAddr LookupPa(VirtAddr va);

  PageDirectory& directory() { return directory_; }
  PhysAddr directory_pa() { return directory_page_->pa; }

 private:
  PageDirectory& directory_;
  PagesRef directory_page_;

  PagesRef page_table_pages_[PageDirectory::kSize];
};

}  // namespace arch
