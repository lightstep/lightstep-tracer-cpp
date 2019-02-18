#pragma once

#include "common/noncopyable.h"

namespace lightstep {
class SatelliteStreamer;

class SatelliteConnection : private Noncopyable {
 public:
  SatelliteConnection(SatelliteStreamer& streamer) noexcept;

 private:
  SatelliteStreamer& streamer_;
};
}  // namespace lightstep
