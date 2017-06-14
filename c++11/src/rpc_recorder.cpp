#include <collector.grpc.pb.h>
#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <thread>
#include <opentracing/stringref.h>
#include "recorder.h"
#include "default_tags.h"
using namespace opentracing;

namespace lightstep {
collector::KeyValue to_key_value(StringRef key, const Value& value);
uint64_t generate_id();

//------------------------------------------------------------------------------
// ReportBuilder
//------------------------------------------------------------------------------
// ReportBuilder helps construct lightstep::collector::ReportRequest
// messages.  Not thread-safe, thread compatible.
namespace {
class ReportBuilder {
public:
 ReportBuilder(const TracerOptions& options) : reset_next_(true) {
   // TODO Fill in any core internal_metrics.
   collector::Reporter *reporter = preamble_.mutable_reporter();
   for (const auto& tag : options.tags)
     *reporter->mutable_tags()->Add() = to_key_value(tag.first, tag.second);
   reporter->set_reporter_id(generate_id());
   preamble_.mutable_auth()->set_access_token(options.access_token);
 }

 // addSpan adds the span to the currently-building ReportRequest.
 void addSpan(collector::Span&& span) {
   if (reset_next_) {
     pending_.Clear();
     pending_.CopyFrom(preamble_);
     reset_next_ = false;
   }
   *pending_.mutable_spans()->Add() = span;
  }

  // pendingSpans() is the number of pending spans.
  size_t pendingSpans() const {
    return pending_.spans_size();
  }

  void setPendingClientDroppedSpans(uint64_t spans) {
    auto count = pending_.mutable_internal_metrics()->add_counts();
    count->set_name("spans.dropped");
    count->set_int_value(spans);
  }

  // pending() returns a mutable object, appropriate for swapping with
  // another ReportRequest object.
  collector::ReportRequest& pending() {
    reset_next_ = true;
    return pending_;
  }

private:
  bool reset_next_;
  collector::ReportRequest preamble_;
  collector::ReportRequest pending_;
};
} // anonymous namespace


//------------------------------------------------------------------------------
// RpcRecorder
//------------------------------------------------------------------------------
namespace {
class RpcRecorder : public Recorder {
 public:
   explicit RpcRecorder(const TracerOptions& options) 
     : builder_(options)
   {}

   void RecordSpan(collector::Span&& span) noexcept override {
   }
  private:
   const TracerOptions options_;

  // Writer state.
  std::mutex write_mutex_;
  std::condition_variable write_cond_;
  bool write_exit_;
  std::thread writer_;

  // Buffer state (protected by write_mutex_).
  ReportBuilder builder_;
  collector::ReportRequest inflight_;
  size_t flushed_seqno_;
  size_t encoding_seqno_;
  size_t dropped_spans_;

  // Collector service stub.
  /* collector::CollectorService::Stub client_; */
};
}

//------------------------------------------------------------------------------
// make_lightstep_recorder
//------------------------------------------------------------------------------
std::unique_ptr<Recorder> make_lightstep_recorder(
    const TracerOptions& options) {
  auto options_new = options;
  for (const auto& tag : default_tags)
    options_new.tags[tag.first] = tag.second;
  return std::unique_ptr<Recorder>(new (std::nothrow) RpcRecorder(options));
}
} // namespace lightstep
