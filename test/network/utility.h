#pragma once

#include "network/fixed_fragment_input_stream.h"

#include <string>
#include <utility>

namespace lightstep {
/**
 * Constructs a fragment stream from c-strings.
 * @param fragments the c-strings.
 * @return a fragment input stream with all the c-strings as fragments.
 */
template <class... Fragments>
FixedFragmentInputStream<sizeof...(Fragments)> MakeFixedFragmentInputStream(
    Fragments... fragments) {
  return FixedFragmentInputStream<sizeof...(Fragments)>{
      MakeFragment(fragments)...};
}

/**
 * Builds a string concatenating all the pieces of a fragment set.
 * @param fragment_input_stream supplies the fragments to concatenate.
 * @return a string concatenating all the fragments.
 */
std::string ToString(const FragmentInputStream& fragment_input_stream);
}  // namespace lightstep
