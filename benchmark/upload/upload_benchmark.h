#pragma once

#include "dummy_satellite.h"
#include "upload_benchmark_report.h"

#include <opentracing/tracer.h>

namespace lightstep {
UploadBenchmarkReport RunUploadBenchmark(
    const upload_benchmark_configuration::UploadBenchmarkConfiguration&
        configuration,
    DummySatellite* satellite, std::shared_ptr<opentracing::Tracer>& tracer);
}  // namespace lightstep
