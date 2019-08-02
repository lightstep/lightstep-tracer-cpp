#pragma once

#include <lightstep/tracer.h>
#include <exception>
#include <sstream>

namespace lightstep {
inline void Concatenate(std::ostringstream& /*oss*/) {}

template <class TFirst, class... TRest>
void Concatenate(std::ostringstream& oss, const TFirst& tfirst,
                 const TRest&... trest) {
  oss << tfirst;
  Concatenate(oss, trest...);
}

class Logger {
 public:
  Logger();

  explicit Logger(
      std::function<void(LogLevel, opentracing::string_view)>&& logger_sink);

  inline void Log(LogLevel level,
                  opentracing::string_view message) noexcept try {
    if (static_cast<int>(level) >= static_cast<int>(level_)) {
      logger_sink_(level, message);
    }
  } catch (const std::exception& /*e*/) {
    // Ignore exceptions.
  }

  inline void log(LogLevel level, const char* message) noexcept {
    Log(level, opentracing::string_view{message});
  }

  template <class... Tx>
  inline void Log(LogLevel level, const Tx&... tx) noexcept try {
    if (static_cast<int>(level) < static_cast<int>(level_)) {
      return;
    }
    std::ostringstream oss;
    Concatenate(oss, tx...);
    Log(level, opentracing::string_view{oss.str()});
  } catch (const std::exception& /*e*/) {
    // Ignore exceptions.
  }

  template <class... Tx>
  inline void Debug(const Tx&... tx) noexcept {
    Log(LogLevel::debug, tx...);
  }

  template <class... Tx>
  inline void Info(const Tx&... tx) noexcept {
    Log(LogLevel::info, tx...);
  }

  template <class... Tx>
  inline void Warn(const Tx&... tx) noexcept {
    Log(LogLevel::warn, tx...);
  }

  template <class... Tx>
  inline void Error(const Tx&... tx) noexcept {
    Log(LogLevel::error, tx...);
  }

  inline void set_level(LogLevel level) noexcept { level_ = level; }

  inline LogLevel level() const noexcept { return level_; }

 private:
  std::function<void(LogLevel, opentracing::string_view)> logger_sink_;
  LogLevel level_ = LogLevel::error;
};
}  // namespace lightstep
