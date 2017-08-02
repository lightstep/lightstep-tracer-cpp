#include <lightstep/transporter.h>
#include "nghttp2_async_transporter.h"

namespace lightstep {
//------------------------------------------------------------------------------
// MakeLightStepAsyncTransporter
//------------------------------------------------------------------------------
std::unique_ptr<LightStepAsyncTransporter> MakeLightStepAsyncTransporter() {
  return std::unique_ptr<LightStepAsyncTransporter>{
      new Nghttp2AsyncTransporter{}};
}
} // namespace lightstep
