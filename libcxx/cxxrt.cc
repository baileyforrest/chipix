#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" void __cxa_pure_virtual() {
  fprintf(stderr, "Pure virtual function called\n");
}

void *operator new(size_t size) {
    return malloc(size);
}

void *operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void *p) {
    free(p);
}

void operator delete(void *p, size_t) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete[](void *p, size_t) {
    free(p);
}
