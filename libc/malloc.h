#pragma once

#include <stddef.h>

void __malloc_init(void);
void* __malloc_alloc_pages(size_t count);
void __malloc_free_page(void* addr);
