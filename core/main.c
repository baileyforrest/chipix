#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/mm.h"
#include "core/tty.h"

#ifdef __linux__
#error "You are not using a cross-compiler"
#endif

void kernel_main(void) {
  tty_init();
  mm_init();

  int* foo;
  int* bar;
  int* baz;

  foo = malloc(sizeof(int));
  bar = malloc(sizeof(int));
  baz = malloc(sizeof(int));

  printf("foo: %p, bar: %p, baz: %p\n", foo, bar, baz);
  free(foo);
  foo = malloc(sizeof(int));
  printf("foo: %p, bar: %p, baz: %p\n", foo, bar, baz);

  free(foo);
  free(baz);
  free(bar);

  foo = malloc(1024);
  printf("foo: %p\n", foo);
}
