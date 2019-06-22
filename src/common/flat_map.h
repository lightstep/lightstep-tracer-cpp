#pragma once

#include <algorithm>
#include <vector>
#include <utility>
#include <functional>

namespace lightstep {
template <class Key, class Value, class Comp = std::less<Key>>
class FlatMap {
  using MapImpl = std::vector<std::pair<Key, Value>>;
 public:
  using iterator = typename MapImpl::const_iterator;

  void insert(Key&& key, Value&& value) {
    auto iter = this->lower_bound(key);
    if (iter->first == key) {
      data_[std::distance(data_.begin(), iter)]->second = std::move(value);
      return;
    }
    data_.emplace(iter, std::move(key), std::move(value));
  }

  template <class T>
  iterator lookup(const T& key) const noexcept {
    auto iter = this->lower_bound(key);
    if (iter->first == key) {
      return iter;
    }
    return data_.end();
  }

  iterator begin() const noexcept { return data_.begin(); }

  iterator end() const noexcept { return data_.end(); }

 private:
   MapImpl data_;

  template <class T>
  iterator lower_bound(const T& key) const noexcept {
    return std::lower_bound(data_.begin(), data_.end(), key,
                            [](const std::pair<Key, Value>& lhs, const T& rhs) {
                              return Comp{}(lhs.first, rhs);
                            });
  }
};
} // namespace lightstep
