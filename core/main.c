#include <assert.h>
#include <stdio.h>

#include "tty.h"

#ifdef __linux__
#error "You are not using a cross-compiler"
#endif

void kernel_main(void) {
  tty_init();

  while (1) {
    printf("Hello\nkernel\nWorld!\n");
  }
}
