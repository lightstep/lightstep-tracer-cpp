#include "common/fragment_input_stream.h"

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
