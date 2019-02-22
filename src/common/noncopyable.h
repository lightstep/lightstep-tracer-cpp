#pragma once

namespace lightstep {
/**
 * Helper class to ensure a class doesn't define unwanted copy methods.
 *
 * Modeled after boost::noncopyable
 * See
 * https://www.boost.org/doc/libs/1_68_0/libs/core/doc/html/core/noncopyable.html
 */
class Noncopyable {
 public:
  Noncopyable(const Noncopyable&) = delete;
  Noncopyable(Noncopyable&&) = delete;

  ~Noncopyable() noexcept = default;

  Noncopyable& operator=(const Noncopyable&) = delete;
  Noncopyable& operator=(Noncopyable&&) = delete;

 protected:
  Noncopyable() noexcept = default;
};
}  // namespace lightstep
