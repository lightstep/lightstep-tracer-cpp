#pragma once

#include <cstddef>
#include <initializer_list>
#include <tuple>

#include "common/function_ref.h"

namespace lightstep {
class FragmentSet {
 public:
  using Callback = bool(void* data, int size);

  FragmentSet() noexcept = default;
  FragmentSet(const FragmentSet&) noexcept = default;
  FragmentSet(FragmentSet&&) noexcept = default;

  virtual ~FragmentSet() noexcept = default;

  FragmentSet& operator=(const FragmentSet&) noexcept = default;
  FragmentSet& operator=(FragmentSet&&) noexcept = default;

  virtual int num_fragments() const noexcept = 0;

  virtual bool ForEachFragment(FunctionRef<Callback> callback) const
      noexcept = 0;
};

std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentSet*> fragment_sets, int n) noexcept;
}  // namespace lightstep
