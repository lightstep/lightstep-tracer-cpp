#include "buffered_recorder.h"
#include <exception>
#include "logger.h"

namespace lightstep {
//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
BufferedRecorder::BufferedRecorder(LightStepTracerOptions options,
                                   std::unique_ptr<Transporter>&& transporter)
    : options_{std::move(options)},
      builder_{options_.access_token, options_.tags},
      transporter_{std::move(transporter)} {
  writer_ = std::thread(&BufferedRecorder::Write, this);
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
BufferedRecorder::~BufferedRecorder() {
  MakeWriterExit();
  writer_.join();
}

//------------------------------------------------------------------------------
// RecordSpan
//------------------------------------------------------------------------------
void BufferedRecorder::RecordSpan(collector::Span&& span) noexcept try {
  std::lock_guard<std::mutex> lock_guard{write_mutex_};
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    dropped_spans_++;
    return;
  }
  builder_.AddSpan(std::move(span));
  if (builder_.num_pending_spans() >= options_.max_buffered_spans) {
    write_cond_.notify_all();
  }
} catch (const std::exception& e) {
  GetLogger().error("Failed to record span: {}", e.what());
}

//------------------------------------------------------------------------------
// FlushWithTimeout
//------------------------------------------------------------------------------
bool BufferedRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration timeout) noexcept {
  // Note: there is no effort made to speed up the flush when
  // requested, it simply waits for the regularly scheduled flush
  // operations to clear out all the presently pending data.
  std::unique_lock<std::mutex> lock{write_mutex_};

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
// Write
//------------------------------------------------------------------------------
void BufferedRecorder::Write() {
  auto next = std::chrono::steady_clock::now() + options_.reporting_period;

  while (WaitForNextWrite(next)) {
    FlushOne();

    auto end = std::chrono::steady_clock::now();
    auto elapsed = end - next;

    if (elapsed > options_.reporting_period) {
      next = end;
    } else {
      next = end + options_.reporting_period - elapsed;
    }
  }
}

//------------------------------------------------------------------------------
// WriteReport
//------------------------------------------------------------------------------
bool BufferedRecorder::WriteReport(const collector::ReportRequest& report) {
  auto response_maybe = transporter_->SendReport(report);
  if (!response_maybe) {
    return false;
  }
  if (options_.verbose) {
    GetLogger().info(R"(Report: resp="{}")",
                     response_maybe->ShortDebugString());
  }
  return true;
}

//------------------------------------------------------------------------------
// FlushOne
//------------------------------------------------------------------------------
void BufferedRecorder::FlushOne() {
  size_t save_dropped;
  size_t save_pending;
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    std::lock_guard<std::mutex> lock_guard{write_mutex_};
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
  bool success = WriteReport(inflight_);
  {
    std::lock_guard<std::mutex> lock_guard{write_mutex_};
    ++flushed_seqno_;
    write_cond_.notify_all();
    inflight_.Clear();

    if (!success) {
      dropped_spans_ += save_dropped + save_pending;
    }
  }
}

//------------------------------------------------------------------------------
// MakeWriterExit
//------------------------------------------------------------------------------
void BufferedRecorder::MakeWriterExit() {
  std::lock_guard<std::mutex> lock_guard{write_mutex_};
  write_exit_ = true;
  write_cond_.notify_all();
}

//------------------------------------------------------------------------------
// WaitForNextWrite
//------------------------------------------------------------------------------
bool BufferedRecorder::WaitForNextWrite(
    const std::chrono::steady_clock::time_point& next) {
  std::unique_lock<std::mutex> lock{write_mutex_};
  write_cond_.wait_until(lock, next, [this]() {
    return this->write_exit_ ||
           this->builder_.num_pending_spans() >= options_.max_buffered_spans;
  });
  return !write_exit_;
}
}  // namespace lightstep
