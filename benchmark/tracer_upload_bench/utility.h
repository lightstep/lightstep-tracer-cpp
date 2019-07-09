#pragma once

#include <tuple>

#include "benchmark/tracer_upload_bench/tracer_upload_bench.pb.h"
#include "span_drop_counter.h"
#include "lightstep/tracer.h"

namespace lightstep {
tracer_upload_bench::Configuration ParseConfiguration(const char* filename);

std::tuple<std::shared_ptr<LightStepTracer>, SpanDropCounter*> MakeTracer(
    const tracer_upload_bench::Configuration& config);
}  // namespace lightstep
