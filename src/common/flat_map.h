#pragma once

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace lightstep {
template <class Key, class Value, class Comp = std::less<Key>>
class FlatMap {
  using MapImpl = std::vector<std::pair<Key, Value>>;

 public:
  using iterator = typename MapImpl::const_iterator;

  void insert(Key&& key, Value&& value) {
    auto iter = this->lower_bound(key);
    if (iter != data_.cend() && iter->first == key) {
      data_[std::distance(data_.cbegin(), iter)].second = std::move(value);
      return;
    }
    data_.emplace(iter, std::move(key), std::move(value));
  }

  template <class T>
  iterator lookup(const T& key) const noexcept {
    auto iter = this->lower_bound(key);
    if (iter != data_.end() && iter->first == key) {
      return iter;
    }
    return data_.end();
  }

  bool empty() const noexcept { return data_.empty(); }

  iterator begin() const noexcept { return data_.begin(); }

  iterator end() const noexcept { return data_.end(); }

  const MapImpl& as_vector() const noexcept { return data_; }

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
}  // namespace lightstep
