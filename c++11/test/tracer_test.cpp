#include <lightstep/tracer.h>
using namespace lightstep;

int main() {
  TracerOptions options;
  auto tracer = make_lightstep_tracer(options);
  return 0;
}
