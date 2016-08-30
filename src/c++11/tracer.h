// -*- Mode: C++ -*-

#include "options.h"
#include "propagation.h"
#include "span.h"
#include "value.h"

#include <memory>
#include <functional>

#ifndef __LIGHTSTEP_TRACER_H__
#define __LIGHTSTEP_TRACER_H__

namespace lightstep {

// Tracer is a handle to a TracerImpl or acts as a No-Op.
class Tracer {
 public:
  explicit Tracer(const ImplPtr &impl) : impl_(impl) { }

  // Constructs a No-Op tracer handle. implementation.
  explicit Tracer(std::nullptr_t) { }

  Span StartSpan(const std::string& operation_name, SpanStartOptions opts = {}) const;

  // GlobalTracer returns the global tracer.
  static Tracer Global();

  // InitGlobalTracer sets the global tracer pointer, returns the
  // former global tracer value.
  static Tracer InitGlobal(Tracer);
  
  // Get the implementation object.
  ImplPtr impl() const { return impl_; }

  // Inject() takes the `sc` SpanContext instance and injects it for
  // propagation within `carrier`. The actual type of `carrier` depends on
  // the value of `format`.
  //
  // OpenTracing defines a common set of `format` values (see BuiltinFormat),
  // and each has an expected carrier type.
  //
  // Returns true on success.
  bool Inject(SpanContext sc, const CarrierFormat& format, const CarrierWriter& writer);

  // Extract() returns a SpanContext instance given `format` and `carrier`.
  //
  // OpenTracing defines a common set of `format` values (see BuiltinFormat),
  // and each has an expected carrier type.
  //
  // Returns a `SpanContext` that is `valid()` on success.
  SpanContext Extract(const CarrierFormat& format, const CarrierReader& reader);

 private:
  ImplPtr impl_;
};

// Recorder is an abstract class for buffering and encoding LightStep
// reports.
class Recorder {
public:
  virtual ~Recorder() { }

  // RecordSpan is called by TracerImpl when a new Span is finished.
  // An rvalue-reference is taken to avoid copying the Span contents.
  virtual void RecordSpan(collector::Span&& span) = 0;

  // Flush is called by the user, indicating for some reason that
  // buffered spans should be flushed.  Returns true if the flush
  // succeeded, false if it timed out.
  virtual bool FlushWithTimeout(Duration timeout) = 0;

  // Flush with an indefinite timeout.
  virtual void Flush() {
    // N.B.: Do not use Duration::max() it will overflow the internals
    // of std::condition_variable::wait_for, for example.
    FlushWithTimeout(std::chrono::hours(24));
    // TODO and panic when it fails? use absolute time instead, to
    // avoid the overflow issue?
  }
};

// ReportBuilder helps construct lightstep::collector::ReportRequest
// messages.  Not thread-safe, thread compatible.
class ReportBuilder {
public:
  ReportBuilder(const TracerImpl &tracer);

  // addSpan adds the span to the currently-building ReportRequest.
  void addSpan(collector::Span&& span) {
    *report_.mutable_spans()->Add() = span;
  }
  // clear resets spans, to begin building a new report.
  void clear() {
    report_.clear_spans();
  }

  const collector::ReportRequest& report() const { return report_; }

private:
  collector::ReportRequest report_;
};

// Create a tracer with user-defined transport.
typedef std::function<std::unique_ptr<Recorder>(const TracerImpl&)> RecorderFactory;

Tracer NewUserDefinedTransportLightStepTracer(const TracerOptions& topts, RecorderFactory rf);

}  // namespace lightstep

#endif // __LIGHTSTEP_TRACER_H__
