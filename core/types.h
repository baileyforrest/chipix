#pragma once

#include <stdint.h>

#include "core/strong-int.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using PhysAddr = StrongInt<uintptr_t>;
using VirtAddr = StrongInt<uintptr_t>;

inline constexpr auto kInvalidPa = PhysAddr(-1);
inline constexpr auto kInvalidVa = VirtAddr(-1);
