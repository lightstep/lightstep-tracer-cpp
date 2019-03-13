#pragma once

#include "network/fixed_fragment_input_stream.h"

#include <string>
#include <utility>

namespace lightstep {
/**
 * Converts a c-string to a fragment.
 * @param s supplies the string to convert.
 * @return a fragment for s.
 */
std::pair<void*, int> MakeFragment(const char* s);

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
 * @param fragment_set supplies the fragments to concatenate.
 * @return a string concatenating all the fragments.
 */
std::string ToString(const FragmentSet& fragment_set);
}  // namespace lightstep
