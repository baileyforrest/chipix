#include "libc/malloc.h"

#include <arch.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libc/dlist.h"
#include "libc/macros.h"

#define TAG_BITS 2
_Static_assert((1 << TAG_BITS) <= _Alignof(uintptr_t));

#define TAG_BITS_MASK ((1 << TAG_BITS) - 1)

typedef struct {
  uintptr_t val;
} TaggedVal;

static uintptr_t tagv_val(const TaggedVal* tv) {
  return tv->val & ~TAG_BITS_MASK;
}

static void tagv_set_val(TaggedVal* tv, uintptr_t val) {
  assert((val & TAG_BITS_MASK) == 0);

  tv->val &= TAG_BITS_MASK;
  tv->val |= (uintptr_t)val;
}

static bool tagv_bit(const TaggedVal* tv, int bit) {
  assert(bit >= 0 && bit < TAG_BITS);
  return !!(tv->val & (1 << bit));
}

static void tagv_set_bit(TaggedVal* tv, int bit, bool val) {
  assert(bit >= 0 && bit < TAG_BITS);

  tv->val &= ~(1 << bit);
  tv->val |= (val << bit);
}

typedef struct {
  TaggedVal size;
} Header;

typedef struct {
  TaggedVal size;
} Footer;

#define BIT_HAS_NEXT 0
#define BIT_USED 1

static uintptr_t header_size(const Header* h) { return tagv_val(&h->size); }

static void header_set_size(Header* h, uintptr_t val) {
  tagv_set_val(&h->size, val);
}

static bool header_has_next(const Header* h) {
  return tagv_bit(&h->size, BIT_HAS_NEXT);
}

static void header_set_has_next(Header* h, bool val) {
  tagv_set_bit(&h->size, BIT_HAS_NEXT, val);
}

static bool header_used(const Header* h) {
  return tagv_bit(&h->size, BIT_USED);
}

static void header_set_used(Header* h, bool val) {
  tagv_set_bit(&h->size, BIT_USED, val);
}

static uintptr_t footer_size(const Footer* f) { return tagv_val(&f->size); }

static void footer_set_size(Footer* f, uintptr_t val) {
  tagv_set_val(&f->size, val);
}
static bool footer_has_next(const Footer* f) {
  return tagv_bit(&f->size, BIT_HAS_NEXT);
}

static void footer_set_has_next(Footer* f, bool val) {
  tagv_set_bit(&f->size, BIT_HAS_NEXT, val);
}

static bool footer_used(const Footer* f) {
  return tagv_bit(&f->size, BIT_USED);
}

static void footer_set_used(Footer* f, bool val) {
  tagv_set_bit(&f->size, BIT_USED, val);
}

static Footer* header_get_footer(const Header* header) {
  return (Footer*)((char*)(header + 1) + header_size(header));
}

static Header* footer_get_header(const Footer* footer) {
  return (Header*)((char*)footer - footer_size(footer)) - 1;
}

static Footer* header_prev_footer(const Header* header) {
  assert(header_has_next(header));
  return (Footer*)header - 1;
}

static Header* footer_next_header(const Footer* footer) {
  assert(footer_has_next(footer));
  return (Header*)(footer + 1);
}

static Header* free_node_header(const Dlist* link) { return (Header*)link - 1; }

// Single free list with first fit allocation.
// TODO(bcf): Use better scheme like free list per size.
Dlist g_free_list;

static Header* alloc_node(size_t size) {
  _Static_assert(sizeof(Header) <= _Alignof(max_align_t));
  // We must add padding so the memory after the header is page aligned.
  size_t pad =
      MAX(size_t, sizeof(Header), _Alignof(max_align_t) - sizeof(Header));
  size_t min_alloc_size = pad + size + sizeof(Footer);
  size_t num_pages = DIV_ROUND_UP(min_alloc_size, PAGE_SIZE);

  char* mem = __malloc_alloc_pages(num_pages);
  if (mem == NULL) {
    return NULL;
  }
  size_t real_size = num_pages * PAGE_SIZE;
  size_t payload_size = real_size - pad - sizeof(Footer);

  Header* header = (void*)(mem + pad);
  header_set_size(header, payload_size);
  header_set_used(header, false);
  header_set_has_next(header, false);

  Footer* footer = header_get_footer(header);
  footer_set_size(footer, payload_size);
  footer_set_used(footer, false);
  footer_set_has_next(footer, false);

  return header;
}

void __malloc_init(void) { dlist_init(&g_free_list); }

// We must set size so the next chunk after this one has the correct alignment.
//
// We want:
// `(size + sizeof(Header) + sizeof(Footer)) % _Alignof(max_align_t) == 0`
//
// We subtract `sizeof(Header) + sizeof(Footer)` because it should not be
// counted in the payload size.
static size_t malloc_size_round(size_t size) {
  size += sizeof(Header) + sizeof(Footer);
  size = ROUND_UP_TO(size, _Alignof(max_align_t));
  size -= sizeof(Header) + sizeof(Footer);
  return size;
}

static void* malloc_impl(size_t size, bool* is_new_pages) {
  *is_new_pages = false;
  size = malloc_size_round(size);

  Header* old_header = NULL;
  Dlist* link;
  DLIST_FOR_EACH(link, &g_free_list) {
    Header* header = free_node_header(link);
    assert(!header_used(header));

    if (header_size(header) >= size) {
      dlist_remove(link);
      old_header = header;
      break;
    }
  }

  if (old_header == NULL) {
    old_header = alloc_node(size);
    if (old_header == NULL) {
      return NULL;
    }
    *is_new_pages = true;
  }

  header_set_used(old_header, true);
  Footer* old_footer = header_get_footer(old_header);

  size_t remain = header_size(old_header) - size;

  // Remaining payload must be large enough to hold the list link.
  size_t min_payload = malloc_size_round(sizeof(Dlist));

  // Not enough remaining space to create another chunk. Just return the whole
  // chunk.
  if (remain < min_payload + sizeof(Header) + sizeof(Footer)) {
    footer_set_used(old_footer, true);
    return old_header + 1;
  }

  header_set_size(old_header, size);

  Footer* new_footer = header_get_footer(old_header);
  footer_set_size(new_footer, size);
  footer_set_has_next(new_footer, true);
  footer_set_used(new_footer, true);

  Header* new_header = footer_next_header(new_footer);
  header_set_size(new_header, remain - (sizeof(Header) + sizeof(Footer)));
  header_set_has_next(new_header, true);
  header_set_used(new_header, false);

  Dlist* new_link = (Dlist*)(new_header + 1);
  dlist_prepend(&g_free_list, new_link);

  uintptr_t ret = (uintptr_t)(old_header + 1);
  assert(ret % _Alignof(max_align_t) == 0);
  return (void*)ret;
}

void* malloc(size_t size) {
  bool is_new_pages;
  return malloc_impl(size, &is_new_pages);
}

static Header* try_coalesce_header(Header* header) {
  if (!header_has_next(header)) {
    return header;
  }

  Footer* prev_footer = header_prev_footer(header);
  if (footer_used(prev_footer)) {
    return header;
  }
  Footer* footer = header_get_footer(header);

  Header* prev_header = footer_get_header(prev_footer);
  assert(!header_used(prev_header));

  size_t new_size = header_size(header) + header_size(prev_header) +
                    sizeof(Header) + sizeof(Footer);
  header_set_size(prev_header, new_size);
  footer_set_size(footer, new_size);

  return prev_header;
}

static Footer* try_coalesce_footer(Footer* footer) {
  if (!footer_has_next(footer)) {
    return footer;
  }

  Header* next_header = footer_next_header(footer);
  if (header_used(next_header)) {
    return footer;
  }
  Header* header = footer_get_header(footer);

  Footer* next_footer = header_get_footer(next_header);
  assert(!footer_used(next_footer));

  size_t new_size = header_size(header) + header_size(next_header) +
                    sizeof(Header) + sizeof(Footer);
  header_set_size(header, new_size);
  footer_set_size(next_footer, new_size);
  return next_footer;
}

void free(void* ptr) {
  if (ptr == NULL) {
    return;
  }

  Header* header = free_node_header((Dlist*)ptr);
  assert(header_used(header));

  Footer* footer = header_get_footer(header);
  assert(footer_used(footer));

  header_set_used(header, false);
  footer_set_used(footer, false);

  header = try_coalesce_header(header);
  footer = try_coalesce_footer(footer);

  // TODO(bcf): Return fully freed pages to kernel.

  Dlist* link = (Dlist*)(header + 1);
  dlist_prepend(&g_free_list, link);
}

void* calloc(size_t nmemb, size_t size) {
  size_t size_bytes = nmemb * size;
  bool is_new_pages;
  void* ret = malloc_impl(size_bytes, &is_new_pages);

#ifndef LIBC_IS_LIBK
  // TODO(bcf): In userspace we can avoid memset if `is_new_pages` is true due
  // to COW zero page.
#endif

  memset(ret, '\0', size_bytes);
  return ret;
}
