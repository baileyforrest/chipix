#include "libc/malloc.h"

#include <arch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "libc/tagged-val.h"
#include "libc/intrusive-list.h"
#include "libc/macros.h"

namespace {

constexpr int kTagBits = 2;
static_assert((1 << kTagBits) <= alignof(uintptr_t));

constexpr int kBitHasNext = 0;
constexpr int kBitUsed = 1;

class Common {
 public:
  size_t size() const { return size_.val(); }
  void set_size(size_t val) { size_.set_val(val); }

  bool has_next() const { return size_.GetBit<kBitHasNext>(); }
  void set_has_next(bool val) { size_.SetBit<kBitHasNext>(val); }

  bool used() const { return size_.GetBit<kBitUsed>(); }
  void set_used(bool val) { size_.SetBit<kBitUsed>(val); }

 private:
  TaggedVal<size_t, kTagBits> size_;
};

struct Footer;

struct Header : public Common {
  Footer* GetFooter() {
    return reinterpret_cast<Footer*>(reinterpret_cast<char*>(this + 1) + size());
  }

  Footer* PrevFooter();
};

struct Footer : public Common {
  Header* GetHeader() {
    return reinterpret_cast<Header*>(reinterpret_cast<char*>(this) - size()) - 1;
  }

  Header* NextHeader() {
    assert(has_next());
    return reinterpret_cast<Header*>(this + 1);
  }
};

Footer* Header::PrevFooter() {
  assert(has_next());
  return reinterpret_cast<Footer*>(this) - 1;
}

Header* FreeNodeHeader(IntrusiveList::Node* node) { return reinterpret_cast<Header*>(node) - 1; }

// Single free list with first fit allocation.
// TODO(bcf): Use better scheme like free list per size.
IntrusiveList g_free_list;

Header* AllocNode(size_t size) {
  static_assert(sizeof(Header) <= alignof(max_align_t));
  // We must add padding so the memory after the header is page aligned.
  size_t pad =
      std::max(sizeof(Header), alignof(max_align_t) - sizeof(Header));
  size_t min_alloc_size = pad + size + sizeof(Footer);
  size_t num_pages = DIV_ROUND_UP(min_alloc_size, PAGE_SIZE);

  char* mem = reinterpret_cast<char*>(__malloc_alloc_pages(num_pages));
  if (mem == nullptr) {
    return nullptr;
  }
  size_t real_size = num_pages * PAGE_SIZE;
  size_t payload_size = real_size - pad - sizeof(Footer);

  auto* header = reinterpret_cast<Header*>(mem + pad);
  header->set_size(payload_size);
  header->set_used(false);
  header->set_has_next(false);

  Footer* footer = header->GetFooter();
  footer->set_size(payload_size);
  footer->set_used(false);
  footer->set_has_next(false);

  return header;
}

// We must set size so the next chunk after this one has the correct alignment.
//
// We want:
// `(size + sizeof(Header) + sizeof(Footer)) % alignof(max_align_t) == 0`
//
// We subtract `sizeof(Header) + sizeof(Footer)` because it should not be
// counted in the payload size.
size_t SizeRound(size_t size) {
  size += sizeof(Header) + sizeof(Footer);
  size = ROUND_UP_TO(size, alignof(max_align_t));
  size -= sizeof(Header) + sizeof(Footer);
  return size;
}

void* MallocImpl(size_t size, bool* is_new_pages) {
  *is_new_pages = false;
  size = SizeRound(size);

  Header* old_header = nullptr;
  for (auto& link : g_free_list) {
    Header* header = FreeNodeHeader(&link);
    assert(!header->used());

    if (header->size() >= size) {
      g_free_list.erase(link);
      old_header = header;
      break;
    }
  }

  if (old_header == nullptr) {
    old_header = AllocNode(size);
    if (old_header == nullptr) {
      return nullptr;
    }
    *is_new_pages = true;
  }

  old_header->set_used(true);
  Footer* old_footer = old_header->GetFooter();

  size_t remain = old_header->size() - size;

  // Remaining payload must be large enough to hold the list link.
  size_t min_payload = SizeRound(sizeof(IntrusiveList::Node));

  // Not enough remaining space to create another chunk. Just return the whole
  // chunk.
  if (remain < min_payload + sizeof(Header) + sizeof(Footer)) {
    old_footer->set_used(true);
    return old_header + 1;
  }

  old_header->set_size(size);

  Footer* new_footer = old_header->GetFooter();
  new_footer->set_size(size);
  new_footer->set_has_next(true);
  new_footer->set_used(true);

  Header* new_header = new_footer->NextHeader();
  new_header->set_size(remain - (sizeof(Header) + sizeof(Footer)));
  new_header->set_has_next(true);
  new_header->set_used(false);

  auto* new_link = reinterpret_cast<IntrusiveList::Node*>(new_header + 1);
  g_free_list.push_front(*new_link);

  uintptr_t ret = (uintptr_t)(old_header + 1);
  assert(ret % alignof(max_align_t) == 0);
  return (void*)ret;
}

Header* TryCoalesceHeader(Header* header) {
  if (!header->has_next()) {
    return header;
  }

  Footer* prev_footer = header->PrevFooter();
  if (prev_footer->used()) {
    return header;
  }
  Footer* footer = header->GetFooter();

  Header* prev_header = prev_footer->GetHeader();
  assert(!prev_header->used());

  size_t new_size = header->size() + prev_header->size() +
                    sizeof(Header) + sizeof(Footer);
  prev_header->set_size(new_size);
  footer->set_size(new_size);

  return prev_header;
}

Footer* TryCoalesceFooter(Footer* footer) {
  if (!footer->has_next()) {
    return footer;
  }

  Header* next_header = footer->NextHeader();
  if (next_header->used()) {
    return footer;
  }
  Header* header = footer->GetHeader();

  Footer* next_footer = next_header->GetFooter();
  assert(!next_footer->used());

  size_t new_size = header->size() + next_header->size() +
                    sizeof(Header) + sizeof(Footer);
  header->set_size(new_size);
  next_footer->set_size(new_size);

  return next_footer;
}

}  // namespace

void* malloc(size_t size) {
  bool is_new_pages;
  return MallocImpl(size, &is_new_pages);
}

void free(void* ptr) {
  if (ptr == nullptr) {
    return;
  }

  Header* header = FreeNodeHeader(reinterpret_cast<IntrusiveList::Node*>(ptr));
  assert(header->used());

  Footer* footer = header->GetFooter();
  assert(footer->used());

  header->set_used(false);
  footer->set_used(false);

  header = TryCoalesceHeader(header);
  footer = TryCoalesceFooter(footer);

  // TODO(bcf): Return fully freed pages to kernel.

  auto* link = reinterpret_cast<IntrusiveList::Node*>(header + 1);
  g_free_list.push_front(*link);
}

void* calloc(size_t nmemb, size_t size) {
  size_t size_bytes = nmemb * size;
  bool is_new_pages;
  void* ret = MallocImpl(size_bytes, &is_new_pages);

#ifndef LIBC_IS_LIBK
  // TODO(bcf): In userspace we can avoid memset if `is_new_pages` is true due
  // to COW zero page.
#endif

  memset(ret, '\0', size_bytes);
  return ret;
}
