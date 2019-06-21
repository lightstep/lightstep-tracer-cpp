#include "common/fragment_array_input_stream.h"

#include <cassert>
#include <iostream>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
FragmentArrayInputStream::FragmentArrayInputStream(
    std::initializer_list<Fragment> fragments)
    : fragments_{fragments} {}

//--------------------------------------------------------------------------------------------------
// operator=
//--------------------------------------------------------------------------------------------------
FragmentArrayInputStream& FragmentArrayInputStream::operator=(
    std::initializer_list<Fragment> fragments) {
  fragments_ = fragments;
  fragment_index_ = 0;
  position_ = 0;
  return *this;
}

//--------------------------------------------------------------------------------------------------
// num_fragments
//--------------------------------------------------------------------------------------------------
int FragmentArrayInputStream::num_fragments() const noexcept {
  return static_cast<int>(fragments_.size()) - fragment_index_;
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool FragmentArrayInputStream::ForEachFragment(Callback callback) const
    noexcept {
  if (fragment_index_ >= static_cast<int>(fragments_.size())) {
    return true;
  }

  auto current_fragment = fragments_[fragment_index_];
  auto result =
      callback(static_cast<void*>(static_cast<char*>(current_fragment.first) +
                                  position_),
               current_fragment.second - position_);
  if (!result) {
    return false;
  }

  for (int i = fragment_index_ + 1; i < static_cast<int>(fragments_.size());
       ++i) {
    if (!callback(fragments_[i].first, fragments_[i].second)) {
      return false;
    }
  }
  return true;
}

//--------------------------------------------------------------------------------------------------
// Clear
//--------------------------------------------------------------------------------------------------
void FragmentArrayInputStream::Clear() noexcept {
  fragment_index_ = 0;
  position_ = 0;
  fragments_.clear();
}

//--------------------------------------------------------------------------------------------------
// Seek
//--------------------------------------------------------------------------------------------------
void FragmentArrayInputStream::Seek(int fragment_index, int position) noexcept {
  assert(fragment_index_ + fragment_index <
         static_cast<int>(fragments_.size()));
  if (fragment_index == 0) {
    position_ += position;
  } else {
    fragment_index_ += fragment_index;
    position_ = position;
  }
  assert(fragment_index_ < static_cast<int>(fragments_.size()));
  assert(position_ < fragments_[fragment_index_].second);
}
}  // namespace lightstep
