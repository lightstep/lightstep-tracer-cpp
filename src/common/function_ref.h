#pragma once

#include <memory>
#include <type_traits>

namespace lightstep {
template <class Sig>
class FunctionRef;

/**
 * Non-owning function reference that can be used as a more performant
 * replacement for std::function when ownership sematics aren't needed.
 *
 * See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0792r0.html
 *
 * Based off of https://stackoverflow.com/a/39087660/4447365
 */
template <class R, class... Args>
class FunctionRef<R(Args...)> {
  void* callable_ = nullptr;
  R (*invoker_)(void*, Args...) = nullptr;

  template <class F>
  using FunctionPointer = decltype(std::addressof(std::declval<F&>()));

  template <class F>
  void BindTo(F& f) noexcept {
    callable_ = static_cast<void*>(std::addressof(f));
    invoker_ = [](void* callable_, Args... args) -> R {
      return (*static_cast<FunctionPointer<F>>(callable_))(
          std::forward<Args>(args)...);
    };
  }

  template <class R_in, class... Args_in>
  void BindTo(R_in (*f)(Args_in...)) noexcept {
    using F = decltype(f);
    if (f == nullptr) {
      return BindTo(nullptr);
    }
    callable_ = reinterpret_cast<void*>(f);
    invoker_ = [](void* callable_, Args... args) -> R {
      return (F(callable_))(std::forward<Args>(args)...);
    };
  }

  void BindTo(std::nullptr_t) noexcept {
    callable_ = nullptr;
    invoker_ = nullptr;
  }

 public:
  template <
      class F,
      typename std::enable_if<
          !std::is_same<FunctionRef, typename std::decay<F>::type>{},
          int>::type = 0,
      typename std::enable_if<
          std::is_convertible<typename std::result_of<F&(Args...)>::type, R>{},
          int>::type = 0>
  FunctionRef(F&& f) {
    BindTo(f);  // not forward
  }

  FunctionRef(std::nullptr_t) {}

  FunctionRef(const FunctionRef&) noexcept = default;
  FunctionRef(FunctionRef&&) noexcept = default;

  R operator()(Args... args) const {
    return invoker_(callable_, std::forward<Args>(args)...);
  }

  explicit operator bool() const { return invoker_; }
};
}  // namespace lightstep
