#pragma once

#include <algorithm>
#include <string>

#include "common/flat_map.h"

#include <opentracing/string_view.h>

namespace lightstep {
struct StringViewComparison {
  bool operator()(opentracing::string_view lhs,
                  opentracing::string_view rhs) const noexcept {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }
};

using BaggageFlatMap = FlatMap<std::string, std::string, StringViewComparison>;
}  // namespace lightstep
