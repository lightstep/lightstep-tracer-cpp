#pragma once

#include "satellite_connection.h"
#include "socket.h"

#include <lightstep/tracer.h>
#include <lightstep/transporter.h>

namespace lightstep {
class SatelliteStreamTransporter : public StreamTransporter {
 public:
  SatelliteStreamTransporter(Logger& logger,
                             const LightStepTracerOptions& options);

  size_t Write(const char* buffer, size_t size) final;

 private:
  SatelliteConnection satellite_connection_;
};
}  // namespace lightstep
