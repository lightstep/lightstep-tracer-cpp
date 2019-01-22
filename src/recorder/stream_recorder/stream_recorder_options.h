#pragma once

#include <cstddef>
#include <chrono>

namespace lightstep {
struct StreamRecorderOptions {
  size_t max_span_buffer_bytes = 1048576;

  std::chrono::microseconds polling_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{10});
};
} // namespace lightstep
