#include "recorder/stream_recorder/host_header.h"

#include <cstdio>
#include <cstring>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
HostHeader::HostHeader(const LightStepTracerOptions& tracer_options) {
  size_t max_host_length = 0;
  for (auto& endpoint : tracer_options.satellite_endpoints) {
    max_host_length = std::max(max_host_length, endpoint.first.size());
  }
  format_ = "Host:%" + std::to_string(max_host_length) + "s\r\n";
  auto header_length =
      5 /*std::strlen("Host:")*/ + max_host_length + 2 /*std::strlen("\r\n")*/;
  header_.resize(
      header_length +
      1);  // Note: Allow for an extra null terminator so we can use snprintf.
}

//--------------------------------------------------------------------------------------------------
// set_host
//--------------------------------------------------------------------------------------------------
void HostHeader::set_host(const char* host) noexcept {
  auto rcode =
      std::snprintf(header_.data(), header_.size(), format_.c_str(), host);
  (void)rcode;
  assert(rcode > 0);
}

//--------------------------------------------------------------------------------------------------
// fragment
//--------------------------------------------------------------------------------------------------
std::pair<void*, int> HostHeader::fragment() const noexcept {
  return {static_cast<void*>(const_cast<char*>(header_.data())),
          static_cast<int>(header_.size()) - 1};
}
}  // namespace lightstep
