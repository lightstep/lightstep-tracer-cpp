#pragma once

#include <cstdint>
#include "test/child_process_handle.h"

namespace lightstep {
class MockDnsServerHandle {
 public:
  MockDnsServerHandle() noexcept = default;

  explicit MockDnsServerHandle(uint16_t port);

 private:
  ChildProcessHandle handle_;
};
}  // namespace lightstep
