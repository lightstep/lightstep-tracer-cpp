#pragma once

#include <opentracing/string_view.h>

namespace lightstep {
// Workaround to https://github.com/opentracing/opentracing-cpp/issues/111
extern const opentracing::string_view SamplingPriorityKey;
}  // namespace lightstep
