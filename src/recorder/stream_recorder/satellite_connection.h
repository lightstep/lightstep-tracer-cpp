#pragma once

#include "common/noncopyable.h"

namespace lightstep {
class SatelliteConnectionManager;

class SatelliteConnection : private Noncopyable {
 public:
  SatelliteConnection(SatelliteConnectionManager& connection_manager) noexcept;

 private:
  SatelliteConnectionManager& connection_manager_;
};
}  // namespace lightstep
