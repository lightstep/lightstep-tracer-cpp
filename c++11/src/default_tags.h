#include <lightstep/version.h>
#include <opentracing/value.h>
#include <opentracing/version.h>
#include <utility>

namespace lightstep {
const std::pair<const char*, opentracing::Value> default_tags[] = {
    {"lightstep.tracer_platform", "c++"},
    {"lightstep.tracer_platform_version", __cplusplus},
    {"lightstep.tracer_version", LIGHTSTEP_VERSION},
    {"lightstep.opentracing_version", OPENTRACING_VERSION}};
}  // namespace lightstep
