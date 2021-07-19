#ifndef LIBCXX_TYPE_TRAITS_H_
#define LIBCXX_TYPE_TRAITS_H_

namespace std {

template <typename T>
struct remove_reference {
  using type = T;
};

template <typename T>
struct remove_reference<T&> {
  using type = T;
};

template <typename T>
struct remove_reference<T&&> {
  using type = T;
};

template <class T>
using remove_reference_t = typename remove_reference<T>::type;

}  // namespace std

#endif  // LIBCXX_TYPE_TRAITS_H_
