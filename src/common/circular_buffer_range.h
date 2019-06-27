#pragma once

#include <cassert>
#include <utility>
#include <iterator>

namespace lightstep {
template <class T>
class CircularBufferRange {
 public:
  CircularBufferRange() noexcept = default;

  CircularBufferRange(T* first1, T* last1, T* first2, T* last2) noexcept
      : first1_{first1}, last1_{last1}, first2_{first2}, last2_{last2} {}

  template <class Callback>
  bool ForEach(Callback callback) const
      noexcept(noexcept(std::declval<Callback>()(std::declval<T>()))) {
    for (auto iter = first1_; iter != last1_; ++iter) {
      if (!callback(*iter)) {
        return false;
      }
    }
    for (auto iter = first2_; iter != last2_; ++iter) {
      if (!callback(*iter)) {
        return false;
      }
    }
    return true;
  }

  size_t size() const noexcept {
    return static_cast<size_t>(std::distance(first1_, last1_) +
                               std::distance(first2_, last2_));
  }

  CircularBufferRange Take(size_t n) const noexcept {
    assert(n <= size());
    if (std::distance(first1_, last1_) >= n) {
      return {first1_, first1_ + n, nullptr, nullptr};
    }
    return {first1_, last1_, first2_, first2_ + n};
  }

 private:
  T* first1_{nullptr};
  T* last1_{nullptr};
  T* first2_{nullptr};
  T* last2_{nullptr};
};
}  // namespace lightstep
