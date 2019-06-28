#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>
#include <utility>

namespace lightstep {
/**
 * A non-owning view into a range of elements in a circular buffer.
 */
template <class T>
class CircularBufferRange {
 public:
  CircularBufferRange() noexcept = default;

  CircularBufferRange(T* first1, T* last1, T* first2, T* last2) noexcept
      : first1_{first1}, last1_{last1}, first2_{first2}, last2_{last2} {}

  operator CircularBufferRange<const T>() const noexcept {
    return {first1_, last1_, first2_, last2_};
  }

  /**
   * Iterate over the elements in the range.
   * @param callback the callback to call for each element
   * @return true if we iterated over all elements
   */
  template <class Callback>
  bool ForEach(Callback callback) const
      noexcept(noexcept(std::declval<Callback>()(std::declval<T&>()))) {
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

  /**
   * @return the number of elements in the range
   */
  size_t size() const noexcept {
    return static_cast<size_t>(std::distance(first1_, last1_) +
                               std::distance(first2_, last2_));
  }

  /**
   * @return true if the range is empty
   */
  bool empty() const noexcept {
    assert((first1_ != last1_) || (first2_ == last2_));
    return first1_ == last1_;
  }

  /**
   * Return a subrange taken from the start of this range.
   * @param n the number of element to take in the subrange
   * @return a subrange of the first n elements in this range
   */
  CircularBufferRange Take(size_t n) const noexcept {
    assert(n <= size());
    auto size1 = std::distance(first1_, last1_);
    if (static_cast<size_t>(size1) >= n) {
      return {first1_, first1_ + n, nullptr, nullptr};
    }
    return {first1_, last1_, first2_, first2_ + (n - size1)};
  }

 private:
  T* first1_{nullptr};
  T* last1_{nullptr};
  T* first2_{nullptr};
  T* last2_{nullptr};
};
}  // namespace lightstep
