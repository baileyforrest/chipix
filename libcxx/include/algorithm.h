#ifndef ALGORITHM_H_
#define ALGORITHM_H_

namespace std {

template <typename T>
const T& max(const T& a, const T& b) {
  return (a < b) ? b : a;
}

}  // namespace std

#endif  // ALGORITHM_H_
