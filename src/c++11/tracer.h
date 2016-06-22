// -*- Mode: C++ -*-

#include "options.h"
#include "span.h"
#include "types.h"
#include "value.h"

#include <memory>

#ifndef __LIGHTSTEP_TRACER_H__
#define __LIGHTSTEP_TRACER_H__

namespace lightstep {

class TracerImpl;

typedef std::shared_ptr<TracerImpl> ImplPtr;

// Tracer is a handle to a TracerImpl or acts as a No-Op.
class Tracer {
 public:
  explicit Tracer(const ImplPtr &impl) : impl_(impl) { }

  // Constructs a No-Op tracer handle. implementation.
  explicit Tracer(std::nullptr_t) { }

  Span StartSpan(const std::string& operation_name) const;

  Span StartSpanWithOptions(const StartSpanOptions& options) const;

  // GlobalTracer returns the global tracer.
  static Tracer Global();

  // InitGlobalTracer sets the global tracer pointer, returns the
  // former global tracer value.
  static Tracer InitGlobal(Tracer);
  
  // Get the implementation object.
  ImplPtr impl() const { return impl_; }

 private:
  ImplPtr impl_;
};

Tracer NewTracer(const TracerOptions& options);
  

// Recorder is a base class that helps with buffering and encoding
// LightStep reports, although it does not offer complete
// functionality.
//
// TODO: The EncodeForTransit API would be better if it supported
// assembling a multi-span buffer one span at a time.  This is
// difficult to achieve with the Thrift wire format currently in use.
// This topic will be revisited after Thrift is replaced by gRPC.
class Recorder {
public:
  virtual ~Recorder() { }

  // RecordSpan is called by TracerImpl when a new Span is finished.
  // An rvalue-reference is taken to avoid copying the Span contents.
  virtual void RecordSpan(lightstep_net::SpanRecord&& span) = 0;

  // Flush is called by the user, indicating for some reason that
  // buffered spans should be flushed.
  virtual void Flush() { }

  // EncodeForTransit encodes the vector of spans as a LightStep
  // report suitable for sending to the Collector.
  //   'tracer' is the Tracer implementation
  //   'spans' is taken as a reference so that it may be swapped
  //           in and out of a temporary for encoding purposes.  It
  //           be cleared after returning.
  //   'func' is provided the encoded report.
  static void EncodeForTransit(const TracerImpl& tracer,
			       std::vector<lightstep_net::SpanRecord>& spans,
			       std::function<void(const uint8_t* bytes, uint32_t len)> func);
};

}  // namespace lightstep

#endif // __LIGHTSTEP_TRACER_H__
