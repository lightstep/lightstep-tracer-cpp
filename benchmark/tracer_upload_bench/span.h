#pragma once

#include "benchmark/tracer_upload_bench/tracer_upload_bench.pb.h"

#include "opentracing/tracer.h"

namespace lightstep {
size_t ComputeSpanSize(const tracer_upload_bench::Configuration& config);

void GenerateSpans(opentracing::Tracer& tracer, const tracer_upload_bench::Configuration& config);
} // namespace lightstep
