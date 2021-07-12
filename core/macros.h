#pragma once

#define PANIC(...)       \
  do {                   \
    printf(__VA_ARGS__); \
    abort();             \
  } while (0)

#define LOG(...) printf(__VA_ARGS__)
