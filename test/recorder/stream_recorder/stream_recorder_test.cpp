#include "recorder/stream_recorder/stream_recorder.h"

#include <memory>

#include "test/counting_metrics_observer.h"
#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/utility.h"
#include "tracer/lightstep_tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("StreamRecorder") {
  std::unique_ptr<MockSatelliteHandle> mock_satellite{new MockSatelliteHandle{
      static_cast<uint16_t>(PortAssignments::StreamRecorderTest)}};

  // Testing stub. More will be added when StreamRecorder is filled out.
  auto logger = std::make_shared<Logger>();
  LightStepTracerOptions tracer_options;
  tracer_options.satellite_endpoints = {
      {"localhost",
       static_cast<uint16_t>(PortAssignments::StreamRecorderTest)}};
  auto metrics_observer = new CountingMetricsObserver{};
  tracer_options.metrics_observer.reset(metrics_observer);

  StreamRecorderOptions recorder_options;
  recorder_options.min_satellite_reconnect_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{100});
  recorder_options.max_satellite_reconnect_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{150});

  recorder_options.flushing_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{100});
  recorder_options.num_satellite_connections = 1;
  auto stream_recorder = new StreamRecorder{*logger, std::move(tracer_options),
                                            std::move(recorder_options)};
  std::unique_ptr<Recorder> recorder{stream_recorder};
  PropagationOptions propagation_options;
  auto tracer = std::make_shared<LightStepTracerImpl>(
      logger, propagation_options, std::move(recorder));

  SECTION("Spans are consumed from the buffer and sent to the satellite.") {
    auto span = tracer->StartSpan("abc");
    span->Finish();
    REQUIRE(!stream_recorder->empty());
    REQUIRE(IsEventuallyTrue(
        [&stream_recorder] { return stream_recorder->empty(); }));
    std::vector<collector::Span> spans;
    REQUIRE(IsEventuallyTrue([&] {
      spans = mock_satellite->spans();
      return !spans.empty();
    }));
    REQUIRE(spans.size() == 1);
    REQUIRE(metrics_observer->num_spans_sent == 1);
  }

  SECTION("Reports are sent to the satellite.") {
    std::vector<collector::ReportRequest> reports;
    REQUIRE(IsEventuallyTrue([&] {
      reports = mock_satellite->reports();
      return !reports.empty();
    }));
  }

  SECTION("Closing a tracer forces any pending spans to be flushed.") {
    auto span = tracer->StartSpan("abc");
    span->Finish();
    REQUIRE(!stream_recorder->empty());
    tracer->Close();
    REQUIRE(stream_recorder->empty());
    std::vector<collector::Span> spans;
    REQUIRE(IsEventuallyTrue([&] {
      spans = mock_satellite->spans();
      return !spans.empty();
    }));
    REQUIRE(spans.size() == 1);
    REQUIRE(metrics_observer->num_spans_sent == 1);
  }

  SECTION("Flush returns with false if the timeout is reached.") {
    mock_satellite.reset(nullptr);
    auto span = tracer->StartSpan("abc");
    span->Finish();
    REQUIRE(!stream_recorder->empty());
    REQUIRE(!stream_recorder->FlushWithTimeout(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::milliseconds{10})));
    REQUIRE(!stream_recorder->empty());
  }
}
