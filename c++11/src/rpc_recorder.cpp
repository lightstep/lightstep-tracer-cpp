#include "recorder.h"

namespace lightstep {
//------------------------------------------------------------------------------
// RpcRecorder
//------------------------------------------------------------------------------
class RpcRecorder : public Recorder {
 public:
   void RecordSpan(collector::Span&& span) noexcept override {
   }
};
} // namespace lightstep
