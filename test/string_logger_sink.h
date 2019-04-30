#pragma once

#include <lightstep/tracer.h>
#include <mutex>
#include <sstream>

namespace lightstep {
/**
 * A log sink that concatenates all records into a string.
 */
class StringLoggerSink {
 public:
  /**
   * Append the given log.
   * @param log_level the level to log at
   * @param message the log message
   */
  void operator()(LogLevel log_level,
                  opentracing::string_view message) noexcept;

  /**
   * @return a string of concatenated log records.
   */
  std::string contents();

 private:
  std::mutex mutex_;
  std::ostringstream oss_;
};
}  // namespace lightstep
