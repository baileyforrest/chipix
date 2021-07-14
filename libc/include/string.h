#ifndef STRING_H_
#define STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void* s1, const void* s2, size_t n);
void* memcpy(void* __restrict dest, const void* __restrict src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
size_t strlen(const char* s);

#ifdef __cplusplus
}
#endif

#endif  // STRING_H_
