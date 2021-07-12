#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/macros.h"
#include "core/mm.h"
#include "core/multiboot.h"
#include "core/tty.h"

#ifdef __linux__
#error "You are not using a cross-compiler"
#endif

void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
  tty_init();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    PANIC("Invalid multiboot magic: %x", magic);
  }

  mm_init(mbd);

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
