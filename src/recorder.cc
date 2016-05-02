#include <iostream> // TODO REMOVE ME

#include <thread>

#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "impl.h"
#include "recorder.h"
#include "options.h"

#include "lightstep_thrift/lightstep_service.h"

#include "thrift/transport/THttpClient.h"
#include "thrift/transport/TSocket.h"
#include "thrift/transport/TSSLSocket.h"

// TODO Replace several abort() calls w/ better error handling.

namespace lightstep {

using namespace lightstep_thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

const char CollectorThriftRpcPath[] = "/_rpc/v1/reports/binary";

const uint64_t MaxSpansPerReport = 2500;
const uint64_t ReportIntervalMillisecs = 2500;
const uint32_t ReportSizeLowerBound = 200;

namespace transport {

typedef std::lock_guard<std::mutex> MutexLock;

class DefaultRecorder : public Recorder {
public:
  DefaultRecorder(const TracerImpl& impl);
  ~DefaultRecorder();

  void RecordSpan(lightstep_thrift::SpanRecord&& span) override;

private:
  void Write();

  void Flush();

  const TracerImpl& tracer_;

  // Protects this Recorder state.
  std::mutex mutex_;

  // Background Flush thread.
  std::thread writer_;
  std::vector<lightstep_thrift::SpanRecord> flushing_spans_;

  // Transport mechanism.
  boost::shared_ptr<TSocket> socket_;
  boost::shared_ptr<TSSLSocketFactory> ssl_factory_;
  boost::shared_ptr<THttpClient> transport_;
};

DefaultRecorder::DefaultRecorder(const TracerImpl& tracer)
  : tracer_(tracer) {
  MutexLock lock(mutex_);

  writer_ = std::thread(&DefaultRecorder::Write, this);

  socket_ = boost::shared_ptr<TSocket>(new TSocket(tracer_.options().collector_host,
						   tracer_.options().collector_port));
  boost::shared_ptr<TTransport> trans = socket_;

  if (tracer_.options().collector_encryption == "tls") {
    initializeOpenSSL();
    ssl_factory_ = boost::shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory(SSLTLS));
    trans = ssl_factory_->createSocket(socket_->getSocketFD());
  }

  transport_ = boost::shared_ptr<THttpClient>(
      new THttpClient(trans, tracer_.options().collector_host, CollectorThriftRpcPath));

  transport_->open();
}

DefaultRecorder::~DefaultRecorder() {
  // TODO Make the thread return.  This will hang indefinitely.
  writer_.join();
}

void DefaultRecorder::RecordSpan(lightstep_thrift::SpanRecord&& span) {
  MutexLock lock(mutex_);
  if (pending_spans_.size() < MaxSpansPerReport) {
    Recorder::RecordSpan(std::move(span));
  } else {
    // TODO Count dropped span.
  }
}

void DefaultRecorder::Write() {
  using namespace std::chrono;
  auto interval = milliseconds(ReportIntervalMillisecs);
  auto next = steady_clock::now() + interval;

  while (true) {
    std::this_thread::sleep_until(next);

    Flush();

    auto end = steady_clock::now();
    auto elapsed = end - next;

    if (elapsed > interval) {
      next = end;
    } else {
      next = end + interval - elapsed;
    }    
  }
}

void DefaultRecorder::Flush() {
  {
    MutexLock lock(mutex_);

    // Note: At present, there cannot be another Flush call
    // outstanding because the Flush API is not exposed.  More
    // synchronization will be needed if flushing_spans_ is pending in
    // another Flush operation.
    if (!flushing_spans_.empty()) abort();
    std::swap(flushing_spans_, pending_spans_);

    if (flushing_spans_.empty()) return;
  }

  EncodeForTransit(tracer_, flushing_spans_, [this](const uint8_t* buffer, uint32_t length) {
      this->transport_->write(buffer, length);
      this->transport_->flush();
    });
}

} // namespace transport

std::unique_ptr<Recorder> NewDefaultRecorder(const TracerImpl& impl) {
  return std::unique_ptr<Recorder>(new transport::DefaultRecorder(impl));
}

void Recorder::RecordSpan(lightstep_thrift::SpanRecord&& span) {
  pending_spans_.emplace_back(std::move(span));
}

void Recorder::EncodeForTransit(const TracerImpl& tracer,
				std::vector<lightstep_thrift::SpanRecord>& spans,
				std::function<void(const uint8_t* bytes, uint32_t len)> func) {

  // TODO Use a more accurate size upper-bound.
  uint32_t size_est = ReportSizeLowerBound * spans.size();
  boost::shared_ptr<TMemoryBuffer> memory(new TMemoryBuffer(size_est));
  boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(memory));

  ReportRequest report;

  // Note: the following construction happens once per report but
  // is static information, could be precomputed.
  Runtime runtime;
  runtime.__set_guid(tracer.runtime_guid());
  runtime.__set_start_micros(tracer.runtime_start_micros());
  runtime.__set_group_name(tracer.component_name());
  std::vector<KeyValue> attrs;
  for (auto it = tracer.runtime_attributes().begin();
       it != tracer.runtime_attributes().end(); ++it) {
    attrs.emplace_back(util::make_kv(it->first, it->second));
  }
  runtime.__set_attrs(std::move(attrs));
  report.__set_runtime(std::move(runtime));

  // Swap the flushing spans into report, serialize, swap back, then
  // clear.  This allows re-use of the spans memory.
  std::swap(report.span_records, spans);
  report.__isset.span_records = true;

  lightstep_thrift::ReportingServiceClient client(proto);
  lightstep_thrift::Auth auth;
  auth.__set_access_token(tracer.access_token());
  client.send_Report(auth, report);

  std::swap(report.span_records, spans);
  spans.clear();

  uint8_t *buffer;
  uint32_t length;
  memory->getBuffer(&buffer, &length);

  func(buffer, length);
}

}  // namespace lightstep
