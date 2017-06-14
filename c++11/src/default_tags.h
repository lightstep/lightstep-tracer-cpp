#include <utility>
#include <opentracing/value.h>

namespace lightstep {
const std::pair<const char*, opentracing::Value> default_tags[] = {
    {"lightstep.tracer_platform", "c++11"}, {"lightstep.tracer_version", ""}};
} // namespace lightstep
