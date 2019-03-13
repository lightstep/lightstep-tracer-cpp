#pragma once

#include <string>

#include "lightstep/tracer.h"

namespace lightstep {
class HostHeader {
 public:
  explicit HostHeader(const LightStepTracerOptions& tracer_options);

  void set_host(const char* host) noexcept;

  std::pair<void*, int> fragment() const noexcept;

 private:
  std::string format_;
  std::vector<char> header_;
};
}  // namespace lightstep
