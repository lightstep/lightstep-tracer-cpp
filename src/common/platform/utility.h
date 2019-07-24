#pragma once

#include <string>

namespace lightstep {
/**
 * @return the name of the executable invoked or c++program if unsuccessful.
 */
std::string GetProgramName();
}  // namespace lightstep
