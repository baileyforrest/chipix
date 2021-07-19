#pragma once

template <typename T, void (*kDestroy)(T*)>
class Ref {
 public:
  Ref() = default;
  explicit Ref(T* val) : val_(val) {}

  ~Ref() { Unref(); }

  Ref(const Ref& other) { *this = other; }
  Ref& operator=(const Ref& other) {
    Unref();
    val_ = other.val_;
    val_->IncRef();
    return *this;
  }

  Ref(Ref&& other) { *this = other; }
  Ref& operator=(Ref&& other) {
    Unref();
    val_ = other.val_;
    other.val_ = nullptr;
    return *this;
  }

  explicit operator bool() { return val_ != nullptr; }
  T* get() { return val_; }
  T* operator->() { return get(); }

 private:
  void Unref() {
    if (val_ == nullptr) {
      return;
    }

    if (val_->DecRef() == 0) {
      kDestroy(val_);
    }
  }

  T* val_ = nullptr;
};
