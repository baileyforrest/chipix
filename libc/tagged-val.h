#pragma once

#include <assert.h>

template <typename T, int kTagBits>
class TaggedVal {
 public:
  static constexpr int kTagBitsMask = (1 << kTagBits) - 1;

  T val() const { return val_ & ~kTagBitsMask; }

  void set_val(T val) {
    assert((val & kTagBitsMask) == 0);

    val_ &= kTagBitsMask;
    val_ |= val;
  }

  template <int kBit>
  bool GetBit() const {
    static_assert(kBit >= 0 && kBit < kTagBits);
    return !!(val_ & (1 << kBit));
  }

  template <int kBit>
  void SetBit(bool val) {
    static_assert(kBit >= 0 && kBit < kTagBits);

    val_ &= ~(1 << kBit);
    val_ |= (val << kBit);
  }

 private:
  T val_ = 0;
};
