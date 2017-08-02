#include <lightstep/transporter.h>
#include <lightstep/tracer.h>
#include <event2/event.h>
#include "qa_bot.h"
#include <cassert>
#include <iostream>
using namespace lightstep;

static void FlushTracerCallback(int /*sock*/, short /*which*/, void* context) {
  std::cout << "arf\n";
  auto tracer = static_cast<LightStepTracer*>(context);
  tracer->Flush();
}

int main() {
  LightStepManualTracerOptions options;
  auto tracer = MakeLightStepTracer(std::move(options));
  assert(tracer);
  auto base = event_base_new();

  auto timer_event =
      event_new(base, -1, EV_PERSIST, FlushTracerCallback, tracer.get());
  timeval tv = {1, 0};
  evtimer_add(timer_event, &tv);
  QABot bot{base};
  event_base_dispatch(base);
  event_del(timer_event);
  event_base_free(base);
  return 0;
}
