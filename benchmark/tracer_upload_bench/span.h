#pragma once

#include "benchmark/tracer_upload_bench/tracer_upload_bench.pb.h"

#include "opentracing/tracer.h"

namespace lightstep {
/**
 * Computes the approximate serialization size of a span in bytes with the given
 * configuration.
 * @param config the benchmark's configuration
 * @return the approximate size of a span
 */
size_t ComputeSpanSize(const tracer_upload_bench::Configuration& config);

/**
 * Generate spans using the given tracer.
 * @param tracer the tracer to generate spans with
 * @param config the configuration describing how to produce spans
 */
void GenerateSpans(opentracing::Tracer& tracer,
                   const tracer_upload_bench::Configuration& config);
}  // namespace lightstep
