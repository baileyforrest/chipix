#ifndef ASSERT_H_
#define ASSERT_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NDEBUG
#define assert(expr) ((void)(expr))
#else
#define assert(expr)                                                          \
  do {                                                                        \
    if (!(expr)) {                                                            \
      fprintf(stderr, "%s:%d: %s: Assertion `" #expr "` failed.\n", __FILE__, \
              __LINE__, __func__);                                            \
      abort();                                                                \
    }                                                                         \
  } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // ASSERT_H_
