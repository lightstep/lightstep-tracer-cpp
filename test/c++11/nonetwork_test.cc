#include <vector>
#include <iostream>
#include <mutex>

#include "tracer.h"
#include "impl.h"

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }

  const char* what() const noexcept override { return what_; }
private:
  const char *what_;
};

class TestRecorder : public lightstep::Recorder {
public:
  explicit TestRecorder(const lightstep::TracerImpl& impl) : encoder_(impl) { }

  virtual void RecordSpan(lightstep_net::SpanRecord&& span) {
    std::lock_guard<std::mutex> lock(mutex_);
    encoder_.recordSpan(std::move(span));
  }
  virtual bool FlushWithTimeout(lightstep::Duration timeout) {
    std::cerr << encoder_.jsonString() << std::endl;
    return true;
  }

  std::mutex mutex_;
  lightstep::JsonEncoder encoder_;
};

int main() {
  try {
    lightstep::TracerOptions topts;

    topts.access_token = "";
    topts.collector_host = "";
    topts.collector_port = 9998;
    topts.collector_encryption = "";
    
    lightstep::Tracer::InitGlobal(NewUserDefinedTransportLightStepTracer(topts, [](const lightstep::TracerImpl& impl) { return std::unique_ptr<lightstep::Recorder>(new TestRecorder(impl)); }));

    // Recorder has shared ownership. Get a pointer.
    auto tracer = lightstep::Tracer::Global();
    auto recorder = tracer.impl()->recorder();

    auto span = lightstep::Tracer::Global().StartSpan("span/parent");
    auto parent = span.context();
    span.Finish();

    auto cspan = lightstep::Tracer::Global().StartSpan("span/child", { ChildOf(parent) });
    cspan.Finish();

    auto child = cspan.context();

    if (child.parent_span_id() == 0 ||
	child.parent_span_id() != parent.span_id()) {
      throw error("parent/child span_id mismatch");
    }

    if (child.trace_id() != parent.trace_id()) {
      throw error("trace_id mismatch");
    }
    
    lightstep::Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }
  std::cerr << "Success!" << std::endl;
  return 0;
}
