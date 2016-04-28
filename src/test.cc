#include <iostream>

#include "lightstep_thrift/lightstep_types.h"

#include "tracer.h"
#include "impl.h"

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
  // TODO
  // topts.binary_transport = [](const uint8_t* buffer, uint32_t length) {
  //   std::cerr << "Wrote " << length << " bytes" << std::endl
  //                         << std::string(reinterpret_cast<const char*>(buffer), length) << std::endl;
  // };
  Tracer::InitGlobal(NewTracer(topts));
  span = Tracer::Global().StartSpanWithOptions(sopts);
  span.Finish();
}
