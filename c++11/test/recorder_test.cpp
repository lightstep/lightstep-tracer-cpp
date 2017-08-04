#include <lightstep/tracer.h>
#include "../src/buffered_recorder.h"
#include "../src/lightstep_tracer_impl.h"
#include "in_memory_transporter.h"
#include "span_generator.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

TEST_CASE("rpc_recorder") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter();
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION(
      "If spans are recorded at a rate significantly slower than "
      "LightStepTracerOptions::reporting_period, then no spans get dropped.") {
    SpanGenerator span_generator(*tracer, 2 * options.reporting_period);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() ==
          span_generator.num_spans_generated());
  }

  SECTION(
      "If spans are recorded at a rate significantly faster than "
      "LightStepTracerOptions::reporting_period, then a fraction of the spans "
      "get successfuly transported.") {
    SpanGenerator span_generator(*tracer, options.reporting_period / 5);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() >
          span_generator.num_spans_generated() / 10);
  }
}
