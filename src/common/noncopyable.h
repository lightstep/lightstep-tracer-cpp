#pragma once

namespace lightstep {
class Noncopyable {
 public:
  Noncopyable() noexcept = default;

  Noncopyable(const Noncopyable&) = delete;
  Noncopyable(Noncopyable&&) = delete;

  ~Noncopyable() noexcept = default;

  Noncopyable& operator=(const Noncopyable&) = delete;
  Noncopyable& operator=(Noncopyable&&) = delete;
};
}  // namespace lightstep
