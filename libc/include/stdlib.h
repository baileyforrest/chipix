#ifndef STDLIB_H_
#define STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

void abort(void);

static inline int abs(int j) { return j >= 0 ? j : -j; }
static inline long labs(long j) { return j >= 0 ? j : -j; }
static inline long long llabs(long long j) { return j >= 0 ? j : -j; }

#ifdef __cplusplus
}
#endif

#endif  // STDLIB_H_
