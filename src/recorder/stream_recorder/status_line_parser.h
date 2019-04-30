#pragma once

#include <string>

#include <opentracing/string_view.h>

namespace lightstep {
class StatusLineParser {
 public:
  void Reset() noexcept;

  void Parse(opentracing::string_view s);

  bool completed() const noexcept { return complete_; }

  int status_code() const noexcept { return status_code_; }

  opentracing::string_view reason() const noexcept { return reason_; }

 private:
  std::string line_;
  bool complete_{false};
  int status_code_;
  opentracing::string_view reason_;

  void ParseFields();
};
}  // namespace lightstep
