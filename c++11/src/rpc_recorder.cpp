#include <collector.grpc.pb.h>
#include <collector.pb.h>
#include <grpc++/create_channel.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <opentracing/stringref.h>
#include <iostream>
#include <sstream>
#include <thread>
#include "recorder.h"
#include "tags.h"
using namespace opentracing;

namespace lightstep {
collector::KeyValue to_key_value(StringRef key, const Value& value);
uint64_t generate_id();
std::string get_program_name();

//------------------------------------------------------------------------------
// hostPortOf
//------------------------------------------------------------------------------
static std::string hostPortOf(const LightStepTracerOptions& options) {
  std::ostringstream os;
  os << options.collector_host << ":" << options.collector_port;
  return os.str();
}

//------------------------------------------------------------------------------
// ReportBuilder
//------------------------------------------------------------------------------
// ReportBuilder helps construct lightstep::collector::ReportRequest
// messages.  Not thread-safe, thread compatible.
namespace {
class ReportBuilder {
 public:
  explicit ReportBuilder(const LightStepTracerOptions& options)
      : reset_next_(true) {
    // TODO(rnickb): Fill in any core internal_metrics.
    collector::Reporter* reporter = preamble_.mutable_reporter();
    for (const auto& tag : options.tags) {
      *reporter->mutable_tags()->Add() = to_key_value(tag.first, tag.second);
    }
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
  size_t pendingSpans() const { return pending_.spans_size(); }

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
}  // anonymous namespace

//------------------------------------------------------------------------------
// RpcRecorder
//------------------------------------------------------------------------------
namespace {
class RpcRecorder : public Recorder {
 public:
  explicit RpcRecorder(LightStepTracerOptions&& options)
      : options_(std::move(options)),
        builder_(options_),
        client_(grpc::CreateChannel(
            hostPortOf(options_),
            (options_.collector_encryption == "tls")
                ? grpc::SslCredentials(grpc::SslCredentialsOptions())
                : grpc::InsecureChannelCredentials())) {
    writer_ = std::thread(&RpcRecorder::write, this);
  }

  ~RpcRecorder() override {
    make_writer_exit();
    writer_.join();
  }

  void RecordSpan(collector::Span&& span) noexcept override {
    std::lock_guard<std::mutex> lock(write_mutex_);
    if (builder_.pendingSpans() >= options_.max_buffered_spans) {
      dropped_spans_++;
      return;
    }
    builder_.addSpan(std::move(span));
    if (builder_.pendingSpans() >= options_.max_buffered_spans) {
      write_cond_.notify_all();
    }
  }

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  void write();
  bool write_report(const collector::ReportRequest& /*report*/);
  void flush_one();

  // Forces the writer thread to exit immediately.
  void make_writer_exit() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    write_exit_ = true;
    write_cond_.notify_all();
  }

  // Waits until either the timeout or the writer thread is forced to
  // exit.  Returns true if it should continue writing, false if it
  // should exit.
  bool wait_for_next_write(const SteadyClock::time_point& next) {
    std::unique_lock<std::mutex> lock(write_mutex_);
    write_cond_.wait_until(lock, next, [this]() {
      return this->write_exit_ ||
             this->builder_.pendingSpans() >= options_.max_buffered_spans;
    });
    return !write_exit_;
  }

  const LightStepTracerOptions options_;

  // Writer state.
  std::mutex write_mutex_;
  std::condition_variable write_cond_;
  bool write_exit_ = false;
  std::thread writer_;

  // Buffer state (protected by write_mutex_).
  ReportBuilder builder_;
  collector::ReportRequest inflight_;
  size_t flushed_seqno_ = 0;
  size_t encoding_seqno_ = 1;
  size_t dropped_spans_ = 0;

  // Collector service stub.
  collector::CollectorService::Stub client_;
};
}  // namespace

//------------------------------------------------------------------------------
// write
//------------------------------------------------------------------------------
void RpcRecorder::write() {
  auto next = SteadyClock::now() + options_.reporting_period;

  while (wait_for_next_write(next)) {
    flush_one();

    auto end = SteadyClock::now();
    auto elapsed = end - next;

    if (elapsed > options_.reporting_period) {
      next = end;
    } else {
      next = end + options_.reporting_period - elapsed;
    }
  }
}

//------------------------------------------------------------------------------
// flush_one
//------------------------------------------------------------------------------
void RpcRecorder::flush_one() {
  size_t seq;
  size_t save_dropped;
  size_t save_pending;
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    std::lock_guard<std::mutex> lock(write_mutex_);
    save_pending = builder_.pendingSpans();
    if (save_pending == 0) {
      return;
    }
    // TODO(rnickb): Compute and set timestamp_offset_micros
    save_dropped = dropped_spans_;
    builder_.setPendingClientDroppedSpans(save_dropped);
    dropped_spans_ = 0;
    std::swap(builder_.pending(), inflight_);
    ++encoding_seqno_;
  }
  bool success = write_report(inflight_);
  {
    std::lock_guard<std::mutex> lock(write_mutex_);
    ++flushed_seqno_;
    write_cond_.notify_all();
    inflight_.Clear();

    if (!success) {
      dropped_spans_ += save_dropped + save_pending;
    }
  }
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool RpcRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {
  // Note: there is no effort made to speed up the flush when
  // requested, it simply waits for the regularly scheduled flush
  // operations to clear out all the presently pending data.
  std::unique_lock<std::mutex> lock(write_mutex_);

  bool has_encoded = builder_.pendingSpans() != 0;

  if (!has_encoded && encoding_seqno_ == 1 + flushed_seqno_) {
    return true;
  }

  size_t wait_seq = encoding_seqno_ - (has_encoded ? 0 : 1);

  return write_cond_.wait_for(lock, timeout, [this, wait_seq]() {
    return this->flushed_seqno_ >= wait_seq;
  });
}

//------------------------------------------------------------------------------
// write_report
//------------------------------------------------------------------------------
bool RpcRecorder::write_report(const collector::ReportRequest& report) {
  grpc::ClientContext context;
  collector::ReportResponse resp;
  context.set_fail_fast(true);
  context.set_deadline(SystemClock::now() + options_.report_timeout);
  grpc::Status status = client_.Report(&context, report, &resp);
  if (!status.ok()) {
    std::cerr << "Report RPC failed: " << status.error_message();
    // TODO(rnickb): Put some of these back into a buffer, etc. (Presently they
    // all
    // drop.)
    return false;
  }
  // TODO(rnickb): Use response.
  return true;
}

//------------------------------------------------------------------------------
// make_lightstep_recorder
//------------------------------------------------------------------------------
std::unique_ptr<Recorder> make_lightstep_recorder(
    const LightStepTracerOptions& options) noexcept try {
  auto options_new = options;

  // Copy over default tags.
  for (const auto& tag : default_tags) {
    options_new.tags[tag.first] = tag.second;
  }

  // Set the component name if provided or default it to the program name.
  if (!options.component_name.empty()) {
    options_new.tags[component_name_key] = options.component_name;
  } else {
    options_new.tags.emplace(component_name_key, get_program_name());
  }

  return std::unique_ptr<Recorder>(new RpcRecorder(std::move(options_new)));
} catch (const std::exception& e) {
  std::cerr << "Failed to initialize LightStep's recorder: " << e.what();
  return nullptr;
}
}  // namespace lightstep
