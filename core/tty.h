#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void tty_init(void);
void tty_putchar(char c);
void tty_write(const char* data, size_t size);

#ifdef __cplusplus
}
#endif
