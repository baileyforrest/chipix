#pragma once

#include <stddef.h>

void tty_init(void);
void tty_putchar(char c);
void tty_write(const char* data, size_t size);
