#pragma once

#include <cstdint>
#include <vector>

#include "configuration-proto/upload_benchmark_configuration.pb.h"

namespace lightstep {
class DummySatellite {
 public:
  virtual ~DummySatellite() = default;

  virtual void Reserve(size_t num_span_ids) = 0;

  virtual std::vector<uint64_t> span_ids() const = 0;

  virtual size_t num_dropped_spans() const noexcept = 0;

  virtual void Close() = 0;
};

std::unique_ptr<DummySatellite> MakeDummySatellite(
    const tracer_configuration::TracerConfiguration& configuration);
}  // namespace lightstep
