#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <new>

#include "core/macros.h"
#include "core/mm.h"
#include "core/tty.h"
#include "third_party/multiboot.h"

#ifdef __linux__
#error "You are not using a cross-compiler"
#endif

struct Foo {
  Foo() { x = 3; }

  int x = 1;
};

Foo global;

extern "C" void kernel_main(multiboot_info_t* mbd, unsigned int magic) {
  TtyInit();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    PANIC("Invalid multiboot magic: %x", magic);
  }

  mm_init(mbd);

  Foo* foo;
  Foo* bar;
  Foo* baz;

  foo = new Foo;
  bar = new Foo;
  baz = new Foo;

  printf("foo: %p, bar: %p, baz: %p\n", foo, bar, baz);
  delete foo;
  foo = new Foo;
  printf("foo: %p, bar: %p, baz: %p\n", foo, bar, baz);

  delete foo;
  delete baz;
  delete bar;

  foo = new Foo;
  printf("foo: %p, %d\n", foo, foo->x);

  int x = 0;
  auto* placement = new (&x) Foo;
  printf("placement: %d\n", placement->x);

  printf("global: %d\n", global.x);
}
