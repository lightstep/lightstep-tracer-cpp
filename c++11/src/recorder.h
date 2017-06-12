#ifndef RECORDER_H
#define RECORDER_H

#include <collector.pb.h>

namespace lightstep {
//------------------------------------------------------------------------------
// Recorder
//------------------------------------------------------------------------------
class Recorder {
 public:
   virtual ~Recorder() = default;

   virtual void RecordSpan(collector::Span&& span) noexcept = 0;
};
} // namespace lightstep

#endif // RECORDER_H
