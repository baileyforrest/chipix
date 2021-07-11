#pragma once

#define __ENSURE_TYPE_int(x) _Generic((x), int : (x))
#define __ENSURE_TYPE_size_t(x) _Generic((x), size_t : (x))

#define __MIN_IMPL(x, y) ((x < y) ? (x) : (y))
#define __MAX_IMPL(x, y) ((x > y) ? (x) : (y))

#define MIN(type, x, y) \
  ((type)__MIN_IMPL(__ENSURE_TYPE_##type(x), __ENSURE_TYPE_##type(y)))

#define MAX(type, x, y) \
  ((type)__MAX_IMPL(__ENSURE_TYPE_##type(x), __ENSURE_TYPE_##type(y)))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define DIV_ROUND_UP(val, divisor) (((val) + (divisor)-1) / (divisor))
#define ROUND_UP_TO(val, to_round) (DIV_ROUND_UP(val, to_round) * to_round)

#define CONTAINER_OF(ptr, type, member) \
  (type*)((char*)(ptr)-offsetof(type, member))
