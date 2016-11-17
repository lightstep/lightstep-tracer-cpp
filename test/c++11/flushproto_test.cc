#include <vector>
#include <iostream>
#include <mutex>
#include <sstream>

#include "lightstep/tracer.h"
#include "lightstep/impl.h"

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }

  const char* what() const noexcept override { return what_; }
private:
  const char *what_;
};

class TestRecorder : public lightstep::Recorder {
public:
  explicit TestRecorder(const lightstep::TracerImpl& impl) : builder_(impl) { }

  virtual void RecordSpan(lightstep::collector::Span&& span) override {
    std::lock_guard<std::mutex> lock(mutex_);
    builder_.addSpan(std::move(span));
  }
  virtual bool FlushWithTimeout(lightstep::Duration timeout) override {
    std::cerr << builder_.pending().DebugString() << std::endl;
    return true;
  }

  std::mutex mutex_;
  lightstep::ReportBuilder builder_;
};

uint64_t my_guids() {
  static uint64_t unsafe = 100;
  return unsafe++;
}

void check_tracer_names() throw(error) {
  // The official value is not exported from gRPC, so hard-code it again:
  const char grpc_path[] = "/lightstep.collector.CollectorService/Report";
  std::stringstream assembled;
  assembled << "/" << lightstep::CollectorServiceFullName() << "/" << lightstep::CollectorMethodName();
  if (assembled.str() != grpc_path) {
    throw error("Incorrect gRPC service and method names hard-coded");
  }
}

int main() {
  try {
    check_tracer_names();

    lightstep::TracerOptions topts;

    topts.access_token = "i_am_an_access_token";
    topts.collector_host = "";
    topts.collector_port = 9998;
    topts.collector_encryption = "";
    topts.tracer_attributes["lightstep.guid"] = "invalid";
    topts.guid_generator = my_guids;
    
    lightstep::Tracer::InitGlobal(NewUserDefinedTransportLightStepTracer(topts, [](const lightstep::TracerImpl& impl) {
	  return std::unique_ptr<lightstep::Recorder>(new TestRecorder(impl));
	}));

    // Recorder has shared ownership. Get a pointer.
    auto tracer = lightstep::Tracer::Global();
    auto recorder = tracer.impl()->recorder();

    auto span = lightstep::Tracer::Global().StartSpan("span/parent");
    const char *v = "value";
    span.SetTag("test1", v);
    span.SetTag("test2", "value");
    span.SetBaggageItem("test", "baggage");
    auto parent = span.context();
    span.Finish();

    auto cspan = lightstep::Tracer::Global().StartSpan("span/child", { ChildOf(parent) });
    cspan.Finish();

    auto child = cspan.context();

    // if (child.parent_span_id() == 0 ||
    // 	child.parent_span_id() != parent.span_id()) {
    //   throw error("parent/child span_id mismatch");
    // }

    if (child.trace_id() != parent.trace_id()) {
      throw error("trace_id mismatch");
    }

    std::vector<std::pair<std::string, std::string>> headers;
    
    if (!lightstep::Tracer::Global().Inject(parent,
					    lightstep::BuiltinCarrierFormat::HTTPHeadersCarrier,
					    lightstep::make_ordered_string_pairs_writer(&headers))) {
      throw error("inject failed");
    }

    lightstep::SpanContext extracted =
      lightstep::Tracer::Global().Extract(lightstep::BuiltinCarrierFormat::HTTPHeadersCarrier,
					  lightstep::make_ordered_string_pairs_reader(headers));

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
    
    lightstep::Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }
  std::cerr << "Success!" << std::endl;
  return 0;
}
