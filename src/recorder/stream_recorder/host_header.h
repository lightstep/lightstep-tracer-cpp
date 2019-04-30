#pragma once

#include <string>

#include "lightstep/tracer.h"

namespace lightstep {
/**
 * Manages the host header in a streaming HTTP POST request.
 */
class HostHeader {
 public:
  explicit HostHeader(const LightStepTracerOptions& tracer_options);

  /**
   * Sets the host to send in the header.
   * @param host the host to set.
   *
   * Note: host must hvae been specified in LightStepTracerOptions.
   */
  void set_host(const char* host) noexcept;

  /**
   * @return the fragment for the host header.
   *
   * Note: The fragment remains the same even if set_host is called with a
   * different host.
   */
  std::pair<void*, int> fragment() const noexcept;

 private:
  std::string format_;
  std::vector<char> header_;
};
}  // namespace lightstep
