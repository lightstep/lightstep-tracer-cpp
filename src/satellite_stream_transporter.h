#pragma once

#include "socket.h"

#include <lightstep/transporter.h>
#include <lightstep/tracer.h>

namespace lightstep {
class SatelliteStreamTransporter : public StreamTransporter {
 public:
   SatelliteStreamTransporter(const LightStepTracerOptions& options);

   size_t Write(const char* buffer, size_t size) final;
   
 private:
   Socket socket_;
};
} // namespace lightstep
