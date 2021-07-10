#include <stdio.h>
#include <stdlib.h>

void abort(void) {
#ifdef LIBC_IS_LIBK
  printf("kernel: panic: abort()\n");
#else
  printf("abort()\n");
#endif
  while (1) {
  }
}
