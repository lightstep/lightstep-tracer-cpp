#pragma once

#include <lightstep/tracer.h>

namespace lightstep {
class Logger {
 public:
  Logger() = default;

  explicit Logger(
      std::function<void(LogLevel, opentracing::string_view)>&& logger_sink)
      : logger_sink_{std::move(logger_sink)} {}

  void debug(opentracing::string_view message) {
    if (logger_sink_) {
      logger_sink_(LogLevel::debug, message);
    }
  }

  void info(opentracing::string_view message) {
    if (logger_sink_) {
      logger_sink_(LogLevel::info, message);
    }
  }

  void error(opentracing::string_view message) {
    if (logger_sink_) {
      logger_sink_(LogLevel::error, message);
    }
  }

 private:
  std::function<void(LogLevel, opentracing::string_view)> logger_sink_;
};
}  // namespace lightstep
