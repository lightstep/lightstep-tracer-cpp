#pragma once

#include "configuration-proto/tracer_configuration.pb.h"

#include <opentracing/tracer.h>

namespace lightstep {
std::shared_ptr<opentracing::Tracer> MakeTracer(
    const configuration_proto::TracerConfiguration& configuration);
}  // namespace lightstep
