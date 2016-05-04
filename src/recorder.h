// -*- Mode: C++ -*-
#ifndef __LIGHTSTEP_RECORDER_H__
#define __LIGHTSTEP_RECORDER_H__

// Defines the Recorder interface which supports encoding and
// transporting LightStep requests to the collector.

#include <mutex>
#include <utility>
#include <vector>

namespace lightstep_thrift {
class SpanRecord;
}  // namespace lightstep_thrift

namespace lightstep {

// CollectorThriftRpcPath is the intended HTTP path for posting
// encoded reports.
extern const char CollectorThriftRpcPath[];

class TracerImpl;

// Recorder is a base class that helps with buffering and encoding
// LightStep reports, although it does not offer complete
// functionality.
class Recorder {
public:
  virtual ~Recorder() { }

  // RecordSpan is called by TracerImpl when a new Span is finished.
  // An rvalue-reference is taken to avoid copying the Span contents.
  virtual void RecordSpan(lightstep_thrift::SpanRecord&& span) = 0;

  // EncodeForTransit encodes the vector of spans as a LightStep
  // report suitable for sending to the Collector.
  //   'tracer' is the Tracer implementation
  //   'spans' is taken as a reference so that it may be swapped
  //           in and out of a temporary for encoding purposes.  It
  //           be cleared after returning.
  //   'func' is provided the encoded report.
  static void EncodeForTransit(const TracerImpl& tracer,
			       std::vector<lightstep_thrift::SpanRecord>& spans,
			       std::function<void(const uint8_t* bytes, uint32_t len)> func);
  
  // TODO Add explicit TracerImpl::Flush() support.
};

// NewDefaultRecorder returns a Recorder implementation that writes to
// LightStep using a background thread that flushes periodically.
std::unique_ptr<Recorder> NewDefaultRecorder(const TracerImpl &impl);

}  // namespace lightstep

#endif // __LIGHTSTEP_RECORDER_H__
