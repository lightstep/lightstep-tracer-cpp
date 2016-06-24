#include <iostream>
#include <chrono>
#include <thread>

#include "tracer.h"
#include "impl.h"

using namespace lightstep;

int main() {
  try {
    Value val1 = 0.54321;
    Value val2 = "pooh";
    Values val3 = { val1, val2 };
    Dictionary val4 = { std::make_pair("hello", val3),
			std::make_pair("whatever", val2) };

    StartSpanOptions sopts;
    sopts.operation_name = "span/test";
    sopts.tags = std::move(val4);

    auto span = Tracer::Global().StartSpanWithOptions(sopts);
    span.Finish();

    TracerOptions topts;
    // TODO these are not used b/c user-defined transport below.
    // topts.access_token = "bfcebc4e1fa7e66d5502a4af87ae854f";
    // topts.collector_host = "collector.lightstep.com";
    // topts.collector_port = 9997;
    // topts.collector_encryption = "tls";

    topts.recorder_factory = [](const TracerImpl& tracer) {
      UserDefinedTransportOptions tropts;
      return lightstep::NewUserDefinedTransport(tracer, tropts);
    };

    Tracer::InitGlobal(NewTracer(topts));

    for (int i = 0; i < 10; i++) {
      span = Tracer::Global().StartSpanWithOptions(sopts);
      span.Finish();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
  }
  return 0;
}
