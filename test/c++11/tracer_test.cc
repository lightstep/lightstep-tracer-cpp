#include <iostream>
#include <chrono>
#include <thread>

#include "impl.h"
#include "recorder.h"
#include "tracer.h"

using namespace lightstep;

int main() {
  try {
    Value val1 = 0.54321;
    Value val2 = "pooh";
    Values val3 = { val1, val2 };
    Dictionary val4 = { std::make_pair("hello", val3),
			std::make_pair("whatever", val2) };

    auto span = Tracer::Global().StartSpan("span/untraced", {
	StartTagOption("hello", std::move(val3)),
	StartTagOption("whatever", std::move(val2)),
      });
    span.Finish();

    TracerOptions topts;

    // topts.access_token = "DEVELOPMENT_TOKEN_jmacd";
    // topts.collector_host = "localhost";
    topts.access_token = "bfcebc4e1fa7e66d5502a4af87ae854f";
    topts.collector_host = "collector.lightstep.com";

    // topts.collector_port = 9998;
    // topts.collector_encryption = "";
    topts.collector_port = 443;
    // topts.collector_port = 9997;
    topts.collector_encryption = "tls";

    BasicRecorderOptions bopts;

    Tracer::InitGlobal(NewLightStepTracer(topts, bopts));

    for (int i = 0; i < 10; i++) {
      span = Tracer::Global().StartSpan("span/test");
      span.Finish();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    span = Tracer::Global().StartSpan("span/parent");
    auto parent = span.context();
    span.Finish();

    auto child = Tracer::Global().StartSpan("span/child", { ChildOf(parent) });
    child.Finish();

    Tracer::Global().impl()->Flush();
  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }
  std::cerr << "Success!" << std::endl;
  return 0;
}
