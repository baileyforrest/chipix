#pragma once

#include <assert.h>
#include <stdbool.h>

typedef struct Dlist {
  struct Dlist* prev;
  struct Dlist* next;
} Dlist;

static inline void dlist_insert_after(Dlist* exist, Dlist* node) {
  node->prev = exist;
  node->next = exist->next;
  exist->next->prev = node;
  exist->next = node;
}

static inline void dlist_insert_before(Dlist* exist, Dlist* node) {
  node->next = exist;
  node->prev = exist->prev;
  exist->prev->next = node;
  exist->prev = node;
}

static inline void dlist_remove(Dlist* node) {
  node->next->prev = node->prev;
  node->prev->next = node->next;

#ifndef NDEBUG
  node->prev = NULL;
  node->next = NULL;
#endif
}

static inline void dlist_init(Dlist* list) {
  list->prev = list;
  list->next = list;
}

static inline bool dlist_is_empty(const Dlist* list) {
  if (list->prev == list) {
    assert(list->next == list);
    return true;
  }

  assert(list->next != list);
  return false;
}

static inline void dlist_append(Dlist* list, Dlist* node) {
  dlist_insert_after(list->next, node);
}

static inline void dlist_prepend(Dlist* list, Dlist* node) {
  dlist_insert_before(list->prev, node);
}

#define DLIST_FOR_EACH(item, list) \
  for (item = (list)->next; item != (list); item = item->next)
