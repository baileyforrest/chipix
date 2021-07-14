#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void __malloc_init(void);
void* __malloc_alloc_pages(size_t count);
void __malloc_free_page(void* addr);

#ifdef __cplusplus
}
#endif
