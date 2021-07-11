#pragma once

void __malloc_init(void);
void* __malloc_alloc_pages(int count);
void __malloc_free_page(void* addr);
