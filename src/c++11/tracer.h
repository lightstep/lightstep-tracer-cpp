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
  
// Recorder is an abstract class for buffering and encoding LightStep
// reports.
class Recorder {
public:
  virtual ~Recorder() { }

  // RecordSpan is called by TracerImpl when a new Span is finished.
  // An rvalue-reference is taken to avoid copying the Span contents.
  virtual void RecordSpan(lightstep_net::SpanRecord&& span) = 0;

  // Flush is called by the user, indicating for some reason that
  // buffered spans should be flushed.
  virtual void Flush() = 0;
};

// Register the factory prior to NewTracer().
void RegisterRecorderFactory(RecorderFactory factory);

// To setup user-defined transport, configure UserDefinedTransportOptions.
class UserDefinedTransportOptions {
public:
  UserDefinedTransportOptions();
  // TODO setting to limit batch size.

  std::function<void(const std::string& report)> callback;
};

// To setup user-defined transport, set the TracerOptions.recorder to one of these:
std::unique_ptr<Recorder> NewUserDefinedTransport(const TracerImpl& impl, const UserDefinedTransportOptions& options);

}  // namespace lightstep

#endif // __LIGHTSTEP_TRACER_H__
