#pragma once

#define PANIC(...)       \
  do {                   \
    printf(__VA_ARGS__); \
    abort();             \
  } while (0)
