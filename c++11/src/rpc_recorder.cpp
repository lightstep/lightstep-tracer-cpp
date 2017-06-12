#include <lightstep/tracer.h>
#include "recorder.h"

namespace lightstep {
//------------------------------------------------------------------------------
// RpcRecorder
//------------------------------------------------------------------------------
namespace {
class RpcRecorder : public Recorder {
 public:
   explicit RpcRecorder(const TracerOptions& options) {}

   void RecordSpan(collector::Span&& span) noexcept override {
   }
};
}

//------------------------------------------------------------------------------
// make_lightstep_recorder
//------------------------------------------------------------------------------
std::unique_ptr<Recorder> make_lightstep_recorder(
    const TracerOptions& options) {
  return std::unique_ptr<Recorder>(new (std::nothrow) RpcRecorder(options));
}
} // namespace lightstep
