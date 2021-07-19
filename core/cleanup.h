#pragma once

#include <utility>

template <typename Func>
class Cleanup {
 public:
  Cleanup() = default;
  explicit Cleanup(Func func) : func_(std::move(func)), should_call_(true) {}
  ~Cleanup() {
    if (should_call_) {
      func_();
    }
  }

  Cleanup(Cleanup&& other) { *this = other; }
  Cleanup& operator=(Cleanup&& other) {
    if (should_call_) {
      func_();
    }
    func_ = std::move(other.func_);
    should_call_ = other.should_call_;
    other.should_call_ = false;
    return *this;
  }

  void Cancel() && { should_call_ = false; }

 private:
  Func func_;
  bool should_call_ = false;
};

template <typename Func>
Cleanup<Func> MakeCleanup(Func func) {
  return Cleanup<Func>(std::move(func));
}
