#pragma once

#include <lightstep/tracer.h>
#include <mutex>
#include <sstream>

namespace lightstep {
class StringLoggerSink {
 public:
  void operator()(LogLevel log_level,
                  opentracing::string_view message) noexcept;

  std::string contents();

 private:
  std::mutex mutex_;
  std::ostringstream oss_;
};
}  // namespace lightstep
