#pragma once

#include <lightstep/tracer.h>
#include <exception>
#include <sstream>

namespace lightstep {
inline void concatenate(std::ostringstream& /*oss*/) {}

template <class TFirst, class... TRest>
void concatenate(std::ostringstream& oss, const TFirst& tfirst,
                 const TRest&... trest) {
  oss << tfirst;
  concatenate(oss, trest...);
}

class Logger {
 public:
  Logger() = default;

  explicit Logger(
      std::function<void(LogLevel, opentracing::string_view)>&& logger_sink)
      : logger_sink_{std::move(logger_sink)} {}

  void log(LogLevel level, opentracing::string_view message) try {
    if (logger_sink_) {
      logger_sink_(level, message);
    }
  } catch (const std::exception& /*e*/) {
    // Ignore exceptions.
  }

  void log(LogLevel level, const char* message) {
    log(level, opentracing::string_view{message});
  }

  template <class... Tx>
  void log(LogLevel level, const Tx&... tx) try {
    std::ostringstream oss;
    concatenate(oss, tx...);
    log(level, opentracing::string_view{oss.str()});
  } catch (const std::exception& /*e*/) {
    // Ignore exceptions.
  }

 private:
  std::function<void(LogLevel, opentracing::string_view)> logger_sink_;
};
}  // namespace lightstep
