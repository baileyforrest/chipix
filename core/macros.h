#pragma once

#define __ENSURE_TYPE_int(x) _Generic((x), int : (x))

#define __MIN_IMPL(x, y) ((x < y) ? (x) : (y))

#define MIN(type, x, y) \
  ((type)__MIN_IMPL(__ENSURE_TYPE_##type(x), __ENSURE_TYPE_##type(y)))
