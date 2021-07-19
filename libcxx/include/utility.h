#ifndef LIBCXX_UTILITY_H_
#define LIBCXX_UTILITY_H_

#include <type_traits>

namespace std {

template <class T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
  return static_cast<typename std::remove_reference<T>::type&&>(t);
}

}  // namespace std

#endif  // LIBCXX_UTILITY_H_
