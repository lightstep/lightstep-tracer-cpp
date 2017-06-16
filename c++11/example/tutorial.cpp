#include <lightstep/tracer.h>
#include <cstdlib> // for std::getenv
#include <cassert>
#include <iostream>
using namespace lightstep;
using namespace opentracing;

int main() {
  TracerOptions options;
  options.component_name = "Tutorial";
  if (const char* access_token = std::getenv("LIGHTSTEP_ACCESS_TOKEN")) {
    options.access_token = access_token;
  } else {
    std::cerr << "You must set the environmental variable "
                 "`LIGHTSTEP_ACCESS_TOKEN` to your access token!\n";
    return -1;
  }
  auto tracer = make_lightstep_tracer(options);
  assert(tracer);

  auto parent_span = tracer->StartSpan("parent");
  assert(parent_span);

  // Create a child span.
  {
    auto child_span =
        tracer->StartSpan("childA", {ChildOf(&parent_span->context())});
    assert(child_span);

    // Set a simple tag.
    child_span->SetTag("simple tag", 123);

    // Set a complex tag.
    child_span->SetTag("complex tag",
                 Values{123, Dictionary{{"abc", 123}, {"xyz", 4.0}}});

    // Log simple values.
    child_span->Log({{"event", "simple log"}, {"abc", 123}});

    // Log complex values.
    child_span->Log({{"event", "complex log"},
                     {"data", Dictionary{{"a", 1}, {"b", Values{1, 2}}}}});

    child_span->Finish();
  }

  parent_span->Finish();
  tracer->Close();

  return 0;
}
