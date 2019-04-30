#include "test/string_logger_sink.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// operator()
//--------------------------------------------------------------------------------------------------
void StringLoggerSink::operator()(LogLevel log_level,
                                  opentracing::string_view message) noexcept {
  std::lock_guard<std::mutex> log_guard{mutex_};
  oss_ << "Level " << static_cast<int>(log_level) << ": " << message << "\n";
}

//--------------------------------------------------------------------------------------------------
// contents
//--------------------------------------------------------------------------------------------------
std::string StringLoggerSink::contents() {
  std::lock_guard<std::mutex> log_guard{mutex_};
  return oss_.str();
}
}  // namespace lightstep
