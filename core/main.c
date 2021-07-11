#include <assert.h>
#include <stdio.h>

#include "tty.h"

#ifdef __linux__
#error "You are not using a cross-compiler"
#endif

void kernel_main(void) {
  tty_init();

  int foo;
  int bar;

  while (1) {
    printf("Hello\nkernel\nWorld! %p %p\n", &foo, &bar);
  }
}
