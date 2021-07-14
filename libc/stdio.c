#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef LIBC_IS_LIBK
#include "core/tty.h"
#endif  // LIBC_IS_LIBK

#include "libc/macros.h"

typedef struct {
  const char* restrict format;
  int remain;
} PrintfState;

static int printf_int(PrintfState* state, bool is_negtive,
                      unsigned long long val, int pad_digits) {
  char buf[32];
  int idx = 0;

  while (val != 0) {
    buf[idx++] = '0' + val % 10;
    val /= 10;
  }

  for (int i = idx; i < pad_digits; ++i) {
    if (putchar('0') == EOF) {
      return -1;
    }
  }

  if (is_negtive) {
    buf[idx++] = '-';
  }

  while (idx > 0) {
    if (state->remain-- == 0) {
      // TODO: Set errno to EOVERFLOW.
      return -1;
    }

    if (putchar(buf[--idx]) == EOF) {
      return -1;
    }
  }

  return 0;
}

static int printf_hex(PrintfState* state, unsigned long long val,
                      int pad_digits) {
  static const char kLut[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  };
  _Static_assert(ARRAY_SIZE(kLut) == 16);

  char buf[32];
  int idx = 0;

  while (val != 0) {
    buf[idx++] = kLut[val % 16];
    val /= 16;
  }

  for (int i = idx; i < pad_digits; ++i) {
    if (putchar('0') == EOF) {
      return -1;
    }
  }

  while (idx > 0) {
    if (state->remain-- == 0) {
      // TODO: Set errno to EOVERFLOW.
      return -1;
    }

    if (putchar(buf[--idx]) == EOF) {
      return -1;
    }
  }

  return 0;
}

int printf(const char* restrict format, ...) {
  va_list args;
  va_start(args, format);

  PrintfState state;
  state.format = format;
  state.remain = INT_MAX;

  while (*state.format != '\0') {
    if (state.remain == 0) {
      // TODO: Set errno to EOVERFLOW.
      return -1;
    }

    if (*state.format != '%') {
      if (putchar(*state.format) == EOF) {
        return -1;
      }
      ++state.format;
      --state.remain;
      continue;
    }

    const char* format_start = state.format++;

    if (*state.format == '%') {
      ++state.format;
      if (putchar('%') == EOF) {
        return -1;
      }
      --state.remain;
      continue;
    }

    if (*state.format == 'd') {
      ++state.format;

      int val = va_arg(args, int);
      unsigned long long ull_val = val < 0 ? -val : val;

      if (printf_int(&state, val < 0, ull_val, 1) < 0) {
        return -1;
      }
      continue;
    }

    if (*state.format == 'x') {
      ++state.format;

      if (printf_hex(&state, va_arg(args, unsigned), 1) < 0) {
        return -1;
      }
      continue;
    }

    if (*state.format == 's') {
      ++state.format;

      const char* str = va_arg(args, const char*);
      while (*str) {
        if (state.remain-- == 0) {
          // TODO: Set errno to EOVERFLOW.
          return -1;
        }

        if (putchar(*(str++)) == EOF) {
          return -1;
        }
      }
      continue;
    }

    if (*state.format == 'p') {
      ++state.format;

      if (state.remain < 2) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }
      if (puts("0x") < 0) {
        return -1;
      }
      state.remain -= 2;

      uintptr_t val = (uintptr_t)va_arg(args, void*);
      if (printf_hex(&state, val, /*pad_digits=*/sizeof(void*) * 2) < 0) {
        return -1;
      }
      continue;
    }

    if (*state.format == 'c') {
      ++state.format;

      // char promotes to int.
      if (putchar(va_arg(args, int)) == EOF) {
        return -1;
      }

      --state.remain;
      continue;
    }

    // Unsupported format, just print the rest of the format.
    state.format = format_start;
    while (*state.format) {
      if (state.remain-- == 0) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
      }

      if (putchar(*(state.format++)) == EOF) {
        return -1;
      }
    }
    break;
  }

  va_end(args);
  return INT_MAX - state.remain;
}

int putchar(int c) {
#ifdef LIBC_IS_LIBK
  {
    char as_char = c;
    TtyWrite(&as_char, 1);
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
