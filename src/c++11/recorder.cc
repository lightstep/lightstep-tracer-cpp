#include <algorithm>
#include <condition_variable>
#include <exception>
#include <thread>
#include <iostream>
#include <sstream>
#include <mutex>
#include <utility>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "collector.grpc.pb.h"
#include "lightstep/impl.h"
#include "lightstep/options.h"
#include "lightstep/recorder.h"
#include "lightstep/tracer.h"
#include "lightstep/util.h"

#include <grpc++/create_channel.h>

namespace lightstep {

std::string hostPortOf(const TracerOptions& options) {
  std::ostringstream os;
  os << options.collector_host << ":" << options.collector_port;
  return os.str();
}

BasicRecorderOptions::BasicRecorderOptions()
  : time_limit(std::chrono::milliseconds(500)),
    span_limit(2000),
    report_timeout(std::chrono::seconds(5)) { }

class BasicRecorder : public Recorder {
public:
  BasicRecorder(const TracerImpl& impl, const BasicRecorderOptions& options);

  ~BasicRecorder() {
    make_writer_exit();
    writer_.join();
  }

  void RecordSpan(collector::Span&& span) override {
    MutexLock lock(write_mutex_);
    if (builder_.pendingSpans() >= options_.span_limit) {
      dropped_spans_++;
      return;
    }
    builder_.addSpan(std::move(span));
    if (builder_.pendingSpans() >= options_.span_limit) {
      write_cond_.notify_all();
    }
  }

  bool FlushWithTimeout(SystemDuration timeout) override;

private:
  void write();
  bool write_report(const collector::ReportRequest&);
  void flush_one();

  // Forces the writer thread to exit immediately.
  void make_writer_exit() {
    MutexLock lock(write_mutex_);
    write_exit_ = true;
    write_cond_.notify_all();
  }

  // Waits until either the timeout or the writer thread is forced to
  // exit.  Returns true if it should continue writing, false if it
  // should exit.
  bool wait_for_next_write(const SteadyClock::time_point &next) {
    std::unique_lock<std::mutex> lock(write_mutex_);
    write_cond_.wait_until(lock, next, [this]() {
  	return this->write_exit_ ||
	       this->builder_.pendingSpans() >= options_.span_limit;
      });
    return !write_exit_;
  }
    
  const TracerImpl& tracer_;
  const BasicRecorderOptions options_;

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
  collector::CollectorService::Stub client_;
};

BasicRecorder::BasicRecorder(const TracerImpl& tracer, const BasicRecorderOptions& options)
  : tracer_(tracer),
    options_(options),
    write_exit_(false),
    builder_(tracer),
    flushed_seqno_(0),
    encoding_seqno_(1),
    dropped_spans_(0),
    client_(grpc::CreateChannel(hostPortOf(tracer.options()),
				(tracer.options().collector_encryption == "tls") ?
				grpc::SslCredentials(grpc::SslCredentialsOptions()) :
				grpc::InsecureChannelCredentials())) {
      writer_ = std::thread(&BasicRecorder::write, this);
    }

void BasicRecorder::write() {
  auto next = SteadyClock::now() + options_.time_limit;

  while (wait_for_next_write(next)) {
    flush_one();

    auto end = SteadyClock::now();
    auto elapsed = end - next;

    if (elapsed > options_.time_limit) {
      next = end;
    } else {
      next = end + options_.time_limit - elapsed;
    }
  }
}

void BasicRecorder::flush_one() {
  size_t seq;
  size_t save_dropped;
  size_t save_pending;
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    MutexLock lock(write_mutex_);
    save_pending = builder_.pendingSpans();
    if (save_pending == 0) {
      return;
    }
    // TODO Compute and set timestamp_offset_micros
    save_dropped = dropped_spans_;
    builder_.setPendingClientDroppedSpans(save_dropped);
    dropped_spans_ = 0;
    std::swap(builder_.pending(), inflight_);
    ++encoding_seqno_;
  }
  bool success = write_report(inflight_);
  {
    MutexLock lock(write_mutex_);
    ++flushed_seqno_;
    write_cond_.notify_all();
    inflight_.Clear();

    if (!success) {
      dropped_spans_ += save_dropped + save_pending;
    }
  }
}

bool BasicRecorder::write_report(const collector::ReportRequest& report) {
  grpc::ClientContext context;
  collector::ReportResponse resp;
  context.set_fail_fast(true);
  context.set_deadline(SystemClock::now() + options_.report_timeout);
  grpc::Status status = client_.Report(&context, report, &resp);
  if (!status.ok()) {
    std::cout << "Report RPC failed: " << status.error_message();
    // TODO Put some of these back into a buffer, etc. (Presently they all drop.)
    return false;
  }
  // TODO Use response.
  return true;
}

bool BasicRecorder::FlushWithTimeout(SystemDuration timeout) {
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

Tracer NewLightStepTracer(const TracerOptions& topts,
			  const BasicRecorderOptions& bopts) {
  ImplPtr impl(new TracerImpl(topts));
  impl->set_recorder(std::unique_ptr<Recorder>(new BasicRecorder(*impl, bopts)));
  return Tracer(impl);
}

}  // namespace lightstep
