#include <iostream>
#include <chrono>
#include <thread>

#include "tracer.h"

using namespace lightstep;

int main() {
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
  topts.access_token = "bfcebc4e1fa7e66d5502a4af87ae854f";
  topts.collector_host = "collector.lightstep.com";
  topts.collector_port = 9997;
  topts.collector_encryption = "tls";

  Tracer::InitGlobal(NewTracer(topts));

  while (true) {
    span = Tracer::Global().StartSpanWithOptions(sopts);
    span.Finish();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
