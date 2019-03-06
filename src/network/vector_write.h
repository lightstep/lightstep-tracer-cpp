#pragma once

#include <tuple>

#include "network/fragment_set.h"

namespace lightstep {
std::tuple<int, int, int> Write(
    int socket, std::initializer_list<const FragmentSet*> fragment_sets);
}  // namespace lightstep
