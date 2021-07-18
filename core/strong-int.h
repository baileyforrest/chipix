#pragma once

template <typename T>
class StrongInt {
 public:
  explicit StrongInt(T val) : val_(val) {}

  T val() const { return val_; }

 private:
  T val_ = 0;
};
