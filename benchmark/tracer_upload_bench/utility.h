#pragma once

#include <tuple>

#include "benchmark/tracer_upload_bench/tracer_upload_bench.pb.h"
#include "lightstep/tracer.h"
#include "span_drop_counter.h"

namespace lightstep {
/**
 * Parse the benchmark's configuration from a file.
 * @param filename the file containing the benchmark's configuration
 * @return the benchmark configuration
 */
tracer_upload_bench::Configuration ParseConfiguration(const char* filename);

/**
 * Construct a tracer with the provided configuration.
 * @param config the configuration for the tracer
 * @return the tracer and dropped span recorder
 */
std::tuple<std::shared_ptr<LightStepTracer>, SpanDropCounter*> MakeTracer(
    const tracer_upload_bench::Configuration& config);
}  // namespace lightstep
