#include <lightstep/tracer.h>
#include <algorithm>
#include "../src/buffered_recorder.h"
#include "../src/lightstep_tracer_impl.h"
#include "in_memory_transporter.h"
#include "span_generator.h"
#include "testing_condition_variable_wrapper.h"

#define CATCH_CONFIG_MAIN
#include <lightstep/catch/catch.hpp>

using namespace lightstep;
using namespace opentracing;

static int LookupSpansDropped(const collector::ReportRequest& report) {
  if (!report.has_internal_metrics()) {
    return 0;
  }
  auto& counts = report.internal_metrics().counts();
  auto iter = std::find_if(counts.begin(), counts.end(),
                           [](const collector::MetricsSample& sample) {
                             return sample.name() == "spans.dropped";
                           });
  if (iter == counts.end()) {
    return 0;
  }
  if (iter->value_case() != collector::MetricsSample::kIntValue) {
    std::cerr << "spans.dropped not of type int\n";
    std::terminate();
  }
  return static_cast<int>(iter->int_value());
}

TEST_CASE("rpc_recorder") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter{};
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION(
      "If spans are recorded at a rate significantly slower than "
      "LightStepTracerOptions::reporting_period, then no spans get dropped.") {
    SpanGenerator span_generator{*tracer, 2 * options.reporting_period};
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() ==
          span_generator.num_spans_generated());
  }

  SECTION(
      "If spans are recorded at a rate significantly faster than "
      "LightStepTracerOptions::reporting_period, then a fraction of the spans "
      "get successfuly transported.") {
    SpanGenerator span_generator{*tracer, options.reporting_period / 5};
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() >
          span_generator.num_spans_generated() / 10);
  }

  SECTION(
      "If the transporter's SendReport function throws, we drop all subsequent "
      "spans.") {
    logger.set_level(spdlog::level::off);
    SpanGenerator span_generator{*tracer, options.reporting_period * 2};
    in_memory_transporter->set_should_throw(true);
    span_generator.Run(std::chrono::milliseconds(250));
    tracer->Close();
    CHECK(in_memory_transporter->spans().size() == 0);
  }
}

TEST_CASE("rpc_recorder2") {
  spdlog::logger logger{"lightstep", spdlog::sinks::stderr_sink_mt::instance()};
  LightStepTracerOptions options;
  options.reporting_period = std::chrono::milliseconds(2);
  options.max_buffered_spans = 5;
  auto in_memory_transporter = new InMemoryTransporter{};
  auto condition_variable = new TestingConditionVariableWrapper{};
  auto recorder = new BufferedRecorder{
      logger, options, std::unique_ptr<Transporter>{in_memory_transporter},
      std::unique_ptr<ConditionVariableWrapper>{condition_variable}};
  auto tracer = std::shared_ptr<opentracing::Tracer>{
      new LightStepTracerImpl{std::unique_ptr<Recorder>{recorder}}};
  CHECK(tracer);

  SECTION(
      "The reporter thread waits until `now() + options.reporting_period`") {
    auto now = condition_variable->Now();
    condition_variable->WaitTillNextEvent();
    auto event = condition_variable->next_event();
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::WaitEvent*>(
              event) != nullptr);
    CHECK(event->timeout() == now + options.reporting_period);
  }

  SECTION(
      "Dropped spans counts get sent in the next ReportRequest, and cleared in "
      "the following ReportRequest") {
    condition_variable->set_block_notify_all(true);
    for (size_t i = 0; i < options.max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    // Check that a NotifyAllEvent gets added when the buffer overflows
    CHECK(dynamic_cast<const TestingConditionVariableWrapper::NotifyAllEvent*>(
              condition_variable->next_event()) != nullptr);

    condition_variable->Step();  // Process NotifyAll Event
    condition_variable->set_block_notify_all(false);
    condition_variable->Step();  // Waits for the first report to be sent

    auto span = tracer->StartSpan("xyz");
    CHECK(span);
    span->Finish();
    // Ensure that the second report gets sent.
    condition_variable->Step();
    condition_variable->Step();

    auto reports = in_memory_transporter->reports();
    CHECK(reports.size() == 2);
    CHECK(LookupSpansDropped(reports[0]) == 1);
    CHECK(reports[0].spans_size() == options.max_buffered_spans);
    CHECK(LookupSpansDropped(reports[1]) == 0);
    CHECK(reports[1].spans_size() == 1);
  }
}
