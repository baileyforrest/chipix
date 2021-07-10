#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef LIBC_IS_LIBK
#include "core/tty.h"
#endif  // LIBC_IS_LIBK

int printf(const char* restrict format, ...) {
  va_list args;
  va_start(args, format);

  int remain = INT_MAX;

  while (*format != '\0') {
    if (remain == 0) {
      // TODO: Set errno to EOVERFLOW.
      return -1;
    }

    if (*format != '%') {
      if (putchar(*format) == EOF) {
        return -1;
      }
      ++format;
      --remain;
      continue;
    }

    const char* format_start = format++;

    if (*format == '%') {
      ++format;
      if (putchar('%') == EOF) {
        return -1;
      }
      --remain;
      continue;
    }

    if (*format == 'd') {
      ++format;

      int val = va_arg(args, int);
      char buf[32];
      int idx = 0;

      while (val != 0) {
        buf[idx++] = '0' + abs(val % 10);
        val /= 10;
      }

      while (idx > 0) {
        if (remain-- == 0) {
          // TODO: Set errno to EOVERFLOW.
          return -1;
        }

        if (putchar(buf[--idx]) == EOF) {
          return -1;
        }
      }
      continue;
    }

    if (*format == 's') {
      ++format;

      const char* str = va_arg(args, const char*);
      while (*str) {
        if (remain-- == 0) {
          // TODO: Set errno to EOVERFLOW.
          return -1;
        }

        if (putchar(*(str++)) == EOF) {
          return -1;
        }
      }
      continue;
    }

    if (*format == 'c') {
      ++format;

      // char promotes to int.
      if (putchar(va_arg(args, int)) == EOF) {
        return -1;
      }

      --remain;
      continue;
    }

    // Unsupported format, just print the rest of the format.
    format = format_start;
    while (*format) {
      if (remain-- == 0) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }

      if (putchar(*(format++)) == EOF) {
        return -1;
      }
    }
    break;
  }

  va_end(args);
  return INT_MAX - remain;
}

int putchar(int c) {
#ifdef LIBC_IS_LIBK
  {
    char as_char = c;
    tty_write(&as_char, 1);
  }
#endif  // LIBC_IS_LIBK

  return c;
}

int puts(const char* s) {
  while (*s != '\0') {
    putchar(*(s++));
  }

  return 0;
}
