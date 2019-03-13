#pragma once

#include <cstddef>
#include <initializer_list>
#include <tuple>

#include "common/function_ref.h"

namespace lightstep {
using Fragment = std::pair<void*, int>;

/**
 * Interface for iterating over sequences of fragments.
 */
class FragmentSet {
 public:
  using Callback = bool(void* data, int size);

  FragmentSet() noexcept = default;
  FragmentSet(const FragmentSet&) noexcept = default;
  FragmentSet(FragmentSet&&) noexcept = default;

  virtual ~FragmentSet() noexcept = default;

  FragmentSet& operator=(const FragmentSet&) noexcept = default;
  FragmentSet& operator=(FragmentSet&&) noexcept = default;

  /**
   * @return the number of fragments in the set.
   */
  virtual int num_fragments() const noexcept = 0;

  /**
   * @return true if the set contains no fragments.
   */
  bool empty() const noexcept { return num_fragments() == 0; }

  /**
   * Iterate over each fragment in the set.
   * @param callback a callback to call for each fragment. The callback can
   * return false to break out of the iteration early.
   * @return false if the iteration was interrupted early.
   */
  virtual bool ForEachFragment(FunctionRef<Callback> callback) const
      noexcept = 0;
};

/**
 * Determine which fragment an index lies in.
 * @param fragment_sets a list of fragment sets.
 * @param n an index into the fragment sets.
 * @return a tuple identifying the fragment set, fragment, and position of n.
 */
std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentSet*> fragment_sets, int n) noexcept;
}  // namespace lightstep
