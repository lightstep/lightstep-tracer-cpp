#include "logger.h"

namespace lightstep {
//------------------------------------------------------------------------------
// GetLogger
//------------------------------------------------------------------------------
spdlog::logger& GetLogger() {
  static auto logger = spdlog::stderr_logger_mt("lightstep");
  return *logger;
}
}  // namespace lightstep
