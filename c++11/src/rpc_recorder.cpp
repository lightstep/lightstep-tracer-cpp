#include <collector.grpc.pb.h>
#include <collector.pb.h>
#include <lightstep/tracer.h>
#include <opentracing/noop.h>
#include <opentracing/string_view.h>
#include <iostream>
#include <sstream>
#include <thread>
#include "grpc_transporter.h"
#include "recorder.h"
#include "report_builder.h"
#include "utility.h"
using namespace opentracing;

namespace lightstep {
//------------------------------------------------------------------------------
// RpcRecorder
//------------------------------------------------------------------------------
namespace {
class RpcRecorder : public Recorder {
 public:
  explicit RpcRecorder(const LightStepTracerOptions& options,
                       std::unique_ptr<Transporter>&& transporter)
      : options_(options),
        builder_(options_),
        transporter_(std::move(transporter)) {
    writer_ = std::thread(&RpcRecorder::write, this);
  }

  RpcRecorder(const RpcRecorder&) = delete;
  RpcRecorder(RpcRecorder&&) = delete;
  RpcRecorder& operator=(const RpcRecorder&) = delete;
  RpcRecorder& operator=(RpcRecorder&&) = delete;

  ~RpcRecorder() override {
    make_writer_exit();
    writer_.join();
  }

  void RecordSpan(collector::Span&& span) noexcept override {
    std::lock_guard<std::mutex> lock(write_mutex_);
    if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
      dropped_spans_++;
      return;
    }
    builder_.AddSpan(std::move(span));
    if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
      write_cond_.notify_all();
    }
  }

  bool FlushWithTimeout(
      std::chrono::system_clock::duration timeout) noexcept override;

 private:
  void write();
  bool write_report(const collector::ReportRequest& report);
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
             this->builder_.num_pending_spans() >= options_.max_buffered_spans;
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
  std::unique_ptr<Transporter> transporter_;
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
  size_t save_dropped;
  size_t save_pending;
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    std::lock_guard<std::mutex> lock(write_mutex_);
    save_pending = builder_.num_pending_spans();
    if (save_pending == 0) {
      return;
    }
    // TODO(rnburn): Compute and set timestamp_offset_micros
    save_dropped = dropped_spans_;
    builder_.set_pending_client_dropped_spans(save_dropped);
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

  bool has_encoded = builder_.num_pending_spans() != 0;

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
  auto response_maybe = transporter_->SendReport(report);
  return static_cast<bool>(response_maybe);
}

//------------------------------------------------------------------------------
// make_rpc_recorder
//------------------------------------------------------------------------------
std::unique_ptr<Recorder> make_rpc_recorder(
    const LightStepTracerOptions& options,
    std::unique_ptr<Transporter>&& transporter) {
  return std::unique_ptr<Recorder>(
      new RpcRecorder(options, std::move(transporter)));
}

//------------------------------------------------------------------------------
// make_lightstep_recorder
//------------------------------------------------------------------------------
std::unique_ptr<Recorder> make_lightstep_recorder(
    const LightStepTracerOptions& options) noexcept try {
  auto transporter = std::unique_ptr<Transporter>{new GrpcTransporter{options}};
  return make_rpc_recorder(options, std::move(transporter));
} catch (const std::exception& e) {
  std::cerr << "Failed to initialize LightStep's recorder: " << e.what();
  return nullptr;
}
}  // namespace lightstep
