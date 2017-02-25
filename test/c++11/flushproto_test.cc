#include <vector>
#include <iostream>
#include <mutex>
#include <sstream>

#include "lightstep/carrier.h"
#include "lightstep/impl.h"
#include "lightstep/tracer.h"

// TODO turn this into a GUnit test.

class error : public std::exception {
public:
  error(const char *what) : what_(what) { }
  error(const std::string& what) : what_(what) { }

  const char* what() const noexcept override { return what_.c_str(); }
private:
  const std::string what_;
};

class TestRecorder : public lightstep::Recorder {
public:
  explicit TestRecorder(const lightstep::TracerImpl& impl) : builder_(impl) { }

  virtual void RecordSpan(lightstep::collector::Span&& span) override {
    std::lock_guard<std::mutex> lock(mutex_);
    builder_.addSpan(std::move(span));
  }
  virtual bool FlushWithTimeout(lightstep::Duration timeout) override {
    request_ = builder_.pending();
    return true;
  }

  std::mutex mutex_;
  lightstep::ReportBuilder builder_;
  lightstep::collector::ReportRequest request_;
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

void check_test_context_against(const lightstep::SpanContext& got, const lightstep::SpanContext &expect) {
    if (got.trace_id() != expect.trace_id()) {
      throw error("got trace_id mismatch");
    }
    if (got.span_id() != expect.span_id()) {
      throw error("got span_id mismatch");
    }
    int baggage_count = 0;
    got.ForeachBaggageItem([&baggage_count](const std::string& key,
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

    if (child.trace_id() != parent.trace_id()) {
      throw error("trace_id mismatch");
    }

    std::vector<std::pair<std::string, std::string>> headers;

    lightstep::BinaryCarrier binary;
    
    if (!lightstep::Tracer::Global().Inject(parent,
					    lightstep::CarrierFormat::HTTPHeadersCarrier,
					    lightstep::make_ordered_string_pairs_writer(&headers))) {
      throw error("inject (text) failed");
    }
    if (!lightstep::Tracer::Global().Inject(parent,
					    lightstep::CarrierFormat::EnvoyProtoCarrier,
					    lightstep::envoy::ProtoWriter(&binary))) {
      throw error("inject (envoy) failed");
    }

    lightstep::SpanContext extracted_text =
      lightstep::Tracer::Global().Extract(lightstep::CarrierFormat::HTTPHeadersCarrier,
					  lightstep::make_ordered_string_pairs_reader(headers));
    lightstep::SpanContext extracted_envoy =
      lightstep::Tracer::Global().Extract(lightstep::CarrierFormat::EnvoyProtoCarrier,
					  lightstep::envoy::ProtoReader(binary));

    check_test_context_against(extracted_text, parent);
    check_test_context_against(extracted_envoy, parent);

    lightstep::Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }

  try {
    auto tracer = lightstep::Tracer::Global();
    auto recorder = dynamic_cast<TestRecorder*>(tracer.impl()->recorder().get());
    auto report = recorder->request_;

    if (report.spans_size() != 2) {
      throw error("wrong span count");
    }
    auto parent = report.spans(0);
    for (const auto& tag : parent.tags()) {
      if (tag.key().substr(0, 4) != "test") {
	throw error("incorrect tag key");
      }
      if (tag.string_value() != "value") {
	throw error("incorrect tag value");
      }
    }

    std::cerr << "LGTM! " << report.DebugString() << std::endl;    
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }

  std::cerr << "Success!" << std::endl;
  return 0;
}
