#pragma once

#include <chrono>
#include <cstddef>

namespace lightstep {
/**
 * Options to tweak the behavior of StreamRecorder.
 *
 * Note: This class isn't exposed to users. It's only used to change the
 * behavior for testing.
 */
struct StreamRecorderOptions {
  // The maximum number of bytes that will be buffered.
  size_t max_span_buffer_bytes = 1048576;

  // The amount of time between executions of StreamRecorder's polling callback.
  //
  // Note: This should be a short duration; otherwise, the recorder can hang the
  // process when exiting.
  std::chrono::microseconds polling_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{10});

  // The amount of time between flushes of StreamRecorder's span buffer.
  std::chrono::microseconds flushing_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::seconds{5});

  // If the span buffer fills past this fraction of its max size, then we do
  // flush the span buffer early.
  double early_flush_threshold = 0.5;
};
}  // namespace lightstep
