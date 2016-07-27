#include <vector>
#include <iostream>
#include <mutex>

#include "tracer.h"
#include "impl.h"

using namespace lightstep;

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }

  const char* what() const noexcept override { return what_; }
private:
  const char *what_;
};

class TestRecorder : public Recorder {
public:
  explicit TestRecorder(const TracerImpl& impl) : encoder_(impl) { }

  virtual void RecordSpan(lightstep_net::SpanRecord&& span) {
    std::lock_guard<std::mutex> lock(mutex_);
    encoder_.recordSpan(std::move(span));
  }
  virtual bool FlushWithTimeout(Duration timeout) {
    std::cerr << encoder_.jsonString() << std::endl;
    return true;
  }

  std::mutex mutex_;
  JsonEncoder encoder_;
};

int main() {
  try {
    TracerOptions topts;

    topts.access_token = "";
    topts.collector_host = "";
    topts.collector_port = 9998;
    topts.collector_encryption = "";
    
    Tracer::InitGlobal(NewUserDefinedTransportLightStepTracer(topts, [](const TracerImpl& impl) {
	  return std::unique_ptr<Recorder>(new TestRecorder(impl));
	}));

    // Recorder has shared ownership. Get a pointer.
    auto tracer = Tracer::Global();
    auto recorder = tracer.impl()->recorder();

    auto span = Tracer::Global().StartSpan("span/parent");
    span.SetBaggageItem("test", "baggage");
    auto parent = span.context();
    span.Finish();

    auto cspan = Tracer::Global().StartSpan("span/child", { ChildOf(parent) });
    cspan.Finish();

    auto child = cspan.context();

    if (child.parent_span_id() == 0 ||
	child.parent_span_id() != parent.span_id()) {
      throw error("parent/child span_id mismatch");
    }

    if (child.trace_id() != parent.trace_id()) {
      throw error("trace_id mismatch");
    }

    std::vector<std::pair<std::string, std::string>> headers;
    
    if (!Tracer::Global().Inject(parent,
				 BuiltinCarrierFormat::HTTPHeadersCarrier,
				 make_ordered_string_pairs_writer(&headers))) {
      throw error("inject failed");
    }

    SpanContext extracted = Tracer::Global().Extract(BuiltinCarrierFormat::HTTPHeadersCarrier,
						     make_ordered_string_pairs_reader(headers));

    if (extracted.trace_id() != parent.trace_id()) {
      throw error("extracted trace_id mismatch");
    }
    if (extracted.span_id() != parent.span_id()) {
      throw error("extracted span_id mismatch");
    }
    int baggage_count = 0;
    extracted.ForeachBaggageItem([&baggage_count](const std::string& key,
						  const std::string& value) {
				   baggage_count++;
				   if (key != "test") {
				     throw error("unexpected baggage key");
				   }
				   if (value != "baggage") {
				     throw error("unexpected baggage value");
				   }
				   return true;
				 });
    if (baggage_count != 1) {
      throw error("unexpected baggage count");
    }
    
    Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }
  std::cerr << "Success!" << std::endl;
  return 0;
}
