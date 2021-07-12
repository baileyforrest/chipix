#pragma once

#define PANIC(...)       \
  do {                   \
    printf(__VA_ARGS__); \
    abort();             \
  } while (0)

#define PANIC_IF(expr, ...) \
  do {                      \
    if (expr) {             \
      PANIC(__VA_ARGS__);   \
    }                       \
  } while (0)

#define LOG(...) printf(__VA_ARGS__)
