#ifndef STDLIB_H_
#define STDLIB_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void abort(void);

static inline int abs(int j) { return j >= 0 ? j : -j; }
static inline long labs(long j) { return j >= 0 ? j : -j; }
static inline long long llabs(long long j) { return j >= 0 ? j : -j; }

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);

#ifdef __cplusplus
}
#endif

#endif  // STDLIB_H_
