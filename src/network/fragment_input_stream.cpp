#include "network/fragment_input_stream.h"

#include <cstring>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeFragment
//--------------------------------------------------------------------------------------------------
Fragment MakeFragment(const char* s) noexcept {
  return {static_cast<void*>(const_cast<char*>(s)),
          static_cast<int>(std::strlen(s))};
}

//--------------------------------------------------------------------------------------------------
// ComputeFragmentPosition
//--------------------------------------------------------------------------------------------------
std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentInputStream*> fragment_input_streams,
    int n) noexcept {
  int fragment_input_stream_index = 0;
  for (auto fragment_input_stream : fragment_input_streams) {
    int fragment_index = 0;
    auto f = [&n, &fragment_index](void* /*data*/, int size) {
      if (n < size) {
        return false;
      }
      ++fragment_index;
      n -= size;
      return true;
    };
    if (!fragment_input_stream->ForEachFragment(f)) {
      return std::make_tuple(fragment_input_stream_index, fragment_index, n);
    }
    ++fragment_input_stream_index;
  }
  return {static_cast<int>(fragment_input_streams.size()), 0, 0};
}

//--------------------------------------------------------------------------------------------------
// Consume
//--------------------------------------------------------------------------------------------------
bool Consume(std::initializer_list<FragmentInputStream*> fragment_input_streams,
             int n) noexcept {
  for (auto fragment_input_stream : fragment_input_streams) {
    int fragment_index = 0;
    auto f = [&n, &fragment_index](void* /*data*/, int size) {
      if (n < size) {
        return false;
      }
      ++fragment_index;
      n -= size;
      return true;
    };
    if (!fragment_input_stream->ForEachFragment(f)) {
      fragment_input_stream->Seek(fragment_index, n);
      return false;
    }
    fragment_input_stream->Clear();
  }
  return true;
}
}  // namespace lightstep
