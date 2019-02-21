#pragma once

#include <cstdint>

#include "test/child_process_handle.h"

namespace lightstep {
class MockSatelliteHandle {
 public:
  MockSatelliteHandle() noexcept = default;

  explicit MockSatelliteHandle(uint16_t port);

 private:
  ChildProcessHandle handle_;
};
}  // namespace lightstep
