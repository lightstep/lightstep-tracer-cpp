#pragma once

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace lightstep {
/**
 * A minimal key-value map built on top of a vector.
 *
 * See
 * https://www.boost.org/doc/libs/1_65_1/doc/html/boost/container/flat_map.html
 * for background on the flat map container.
 */
template <class Key, class Value, class Comp = std::less<Key>>
class FlatMap {
  using MapImpl = std::vector<std::pair<Key, Value>>;

 public:
  using iterator = typename MapImpl::const_iterator;

  /**
   * Analogous to std::map::insert_or_assign.
   *
   * Inserts a given key-value into the map. Replaces the exiting value if the
   * key already exists.
   * @param key the key to insert
   * @param value the value to insert
   * @return an iterator to the new element
   */
  iterator insert_or_assign(Key&& key, Value&& value) {
    auto iter = this->lower_bound(key);
    if (iter != data_.cend() && iter->first == key) {
      data_[std::distance(data_.cbegin(), iter)].second = std::move(value);
      return iter;
    }
    return data_.emplace(iter, std::move(key), std::move(value));
  }

  /**
   * Analogous to std::map::find.
   *
   * Finds the value with the given key.
   * @param key the key to find
   * @return an iterator pointing to a value with the given key or an end
   * iterator if no such key exists.
   */
  template <class T>
  iterator find(const T& key) const noexcept {
    auto iter = this->lower_bound(key);
    if (iter != data_.end() && iter->first == key) {
      return iter;
    }
    return data_.end();
  }

  /**
   * @return true if the map is empty.
   */
  bool empty() const noexcept { return data_.empty(); }

  /**
   * @return an iterator to the first element in the map.
   */
  iterator begin() const noexcept { return data_.begin(); }

  /**
   * An iterator to the last element in the map.
   */
  iterator end() const noexcept { return data_.end(); }

  /**
   * @return the key-values of hte map as a vector.
   */
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
