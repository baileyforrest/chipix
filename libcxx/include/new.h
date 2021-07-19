#ifndef LIBCXX_NEW_H_
#define LIBCXX_NEW_H_

inline void *operator new(size_t, void *p) throw() { return p; }
inline void *operator new[](size_t, void *p) throw() { return p; }
inline void operator delete(void *, void *) throw() {}
inline void operator delete[](void *, void *) throw() {}

#endif  // LIBCXX_NEW_H_
