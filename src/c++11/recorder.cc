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

#include "impl.h"
#include "recorder.h"
#include "options.h"
#include "tracer.h"
#include "util.h"

namespace lightstep {

const char CollectorJsonRpcPath[] = "/api/v0/reports";

std::string urlOf(const TracerOptions& options) {
  std::ostringstream os;
  os << (options.collector_encryption == "tls" ? "https" : "http") << "://"
     << options.collector_host << ":" << options.collector_port << CollectorJsonRpcPath;
  return os.str();
}

BasicRecorderOptions::BasicRecorderOptions()
  : time_limit(std::chrono::seconds(1)),
    span_limit(1000) { }

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

  bool FlushWithTimeout(Duration timeout) override;

private:
  void write();
  void write_report(const collector::ReportRequest&);
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
  bool wait_for_next_write(const Clock::time_point &next) {
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

  // @@@ GRPC
};

BasicRecorder::BasicRecorder(const TracerImpl& tracer, const BasicRecorderOptions& options)
  : tracer_(tracer),
    options_(options),
    write_exit_(false),
    writer_(std::thread(&BasicRecorder::write, this)),
    builder_(tracer),
    flushed_seqno_(0),
    encoding_seqno_(1),
    dropped_spans_(0) {}

void BasicRecorder::write() {
  auto next = Clock::now() + options_.time_limit;

  while (wait_for_next_write(next)) {
    flush_one();

    auto end = Clock::now();
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
  {
    // Swap the pending encoder with the inflight encoder, then use
    // inflight without a lock. Assumption is that this thread is the
    // only place inflight_ is used.
    MutexLock lock(write_mutex_);
    if (builder_.pendingSpans() == 0) {
      return;
    }
    // TODO Fill in any internal_metrics.
    // TODO Compute and set timestamp_offset_micros
    std::swap(builder_.pending(), inflight_);
    ++encoding_seqno_;
  }
  write_report(inflight_);
  {
    MutexLock lock(write_mutex_);
    ++flushed_seqno_;
    write_cond_.notify_all();
    inflight_.Clear();
  }
}

void BasicRecorder::write_report(const collector::ReportRequest& report) {
  // request_ << boost::network::header("LightStep-Access-Token", tracer.options().access_token);
  // try {
  //   boost::network::http::client::response response = client_.post(request_, report, std::string("application/json"));
  //   if (status(response) != 200) {
  //     std::cerr << "HTTP Status: " << status(response) << " " << status_message(response) << std::endl;
  //   }
  // } catch (std::exception &e) {
  //   std::cerr << "HTTP Exception: " << e.what() << std::endl;
  // }
}

bool BasicRecorder::FlushWithTimeout(Duration timeout) {
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
