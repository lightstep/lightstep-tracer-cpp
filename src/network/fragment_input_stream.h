#pragma once

#include <array>
#include <cassert>
#include <utility>
#include <initializer_list>

#include "network/fragment_set.h"

namespace lightstep {
template <size_t N>
class FragmentInputStream : public FragmentSet {
 public:
   explicit FragmentInputStream(std::initializer_list<std::pair<void*, size_t>> fragments)
     : fragments_{fragments} {
       assert(fragments.size() == N);
   }

   void Seek(int fragment_index, int position) const noexcept {
     assert(fragment_index < N);
     assert(position < fragments_[fragment_index].second);
     fragment_index_ = fragment_index;
     position_ = position;
   }

   void Clear() {
     fragment_index_ = static_cast<int>(N);
   }

   // FragmentSet
   int num_fragments() const noexcept override {
     return static_cast<int>(N) - fragment_index_;
   }

   bool ForEachFragment(Callback callback, void* context) const
       noexcept override {
     if (fragment_index_ >= N) {
       return true;
     }

     auto current_fragment = fragments_[fragment_index_];
     auto result =
         callback(static_cast<void*>(
                      static_cast<char*>(current_fragment.first) + position_),
                  current_fragment.second - position_, context);
     if (!result) {
       return false;
     }

     for (int i = fragment_index_ + 1; i < static_cast<int>(N); ++i) {
       if (!callback(fragments_[i].first, fragments_[i].second, context)) {
         return false;
       }
     }
     return true;
   }

  private:
   std::array<std::pair<void*, int>, N> fragments_;
   int fragment_index_{0};
   int position_{0};
};
} // namespace lightstep
