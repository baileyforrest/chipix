#pragma once

template <typename T>
class StrongInt {
 public:
  StrongInt() = default;
  explicit StrongInt(T val) : val_(val) {}

  T val() const { return val_; }

 private:
  T val_ = 0;
};

template <typename T>
bool operator==(StrongInt<T> lhs, StrongInt<T> rhs) {
  return lhs.val() == rhs.val();
}

template <typename T>
bool operator!=(StrongInt<T> lhs, StrongInt<T> rhs) {
  return lhs.val() != rhs.val();
}

template <typename T, typename T2>
StrongInt<T> operator+(StrongInt<T> lhs, T2 rhs) {
  return StrongInt<T>(lhs.val() + rhs);
}

template <typename T>
bool operator<(StrongInt<T> lhs, StrongInt<T> rhs) {
  return lhs.val() < rhs.val();
}
