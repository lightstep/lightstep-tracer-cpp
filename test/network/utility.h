#pragma once

#include "network/fragment_input_stream.h"

#include <string>
#include <utility>

namespace lightstep {
std::pair<void*, int> MakeFragment(const char* s);

template <class... Fragments>
FragmentInputStream<sizeof...(Fragments)> MakeFragmentInputStream(
    Fragments... fragments) {
  return FragmentInputStream<sizeof...(Fragments)>{MakeFragment(fragments)...};
}

std::string ToString(const FragmentSet& fragment_set);
}  // namespace lightstep
