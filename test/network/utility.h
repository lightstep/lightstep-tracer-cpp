#pragma once

#include "network/fragment_input_stream.h"

#include <string>
#include <utility>

namespace lightstep {
/**
 * Builds a string concatenating all the pieces of a fragment set.
 * @param fragment_input_stream supplies the fragments to concatenate.
 * @return a string concatenating all the fragments.
 */
std::string ToString(const FragmentInputStream& fragment_input_stream);
}  // namespace lightstep
