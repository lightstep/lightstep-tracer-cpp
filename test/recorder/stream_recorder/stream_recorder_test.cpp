#include "recorder/stream_recorder/stream_recorder.h"

#include <atomic>
#include <cstdlib>
#include <memory>
#include <thread>

#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/string_logger_sink.h"
#include "test/utility.h"
#include "tracer/counting_metrics_observer.h"
#include "tracer/legacy/legacy_tracer_impl.h"
#include "tracer/tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

static void GenerateSpans(std::atomic<bool>& stop,
                          opentracing::Tracer& tracer) {
  std::string payload(100000, 'X');
  while (!stop) {
    auto span = tracer.StartSpan("abc");
    span->SetTag("payload", payload.c_str());
  }
}

TEST_CASE("StreamRecorder") {
  std::unique_ptr<MockSatelliteHandle> mock_satellite{new MockSatelliteHandle{
      static_cast<uint16_t>(PortAssignments::StreamRecorderTest)}};

  auto logger_sink = std::make_shared<StringLoggerSink>();
  auto logger = std::make_shared<Logger>(
      [logger_sink](LogLevel log_level, opentracing::string_view message) {
        (*logger_sink)(log_level, message);
      });
  logger->set_level(LogLevel::debug);
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

  tracer_options.reporting_period =
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(
          std::chrono::milliseconds{300});
  recorder_options.num_satellite_connections = 1;
  recorder_options.satellite_graceful_stream_shutdown_timeout =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{300});

  auto stream_recorder = new StreamRecorder{*logger, std::move(tracer_options),
                                            std::move(recorder_options)};
  std::unique_ptr<Recorder> recorder{stream_recorder};
  auto tracer = std::make_shared<TracerImpl>(logger, PropagationOptions{},
                                             std::move(recorder));

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
            std::chrono::nanoseconds{1})));
    REQUIRE(!stream_recorder->empty());
  }

  SECTION("Flush returns immediately if nothing is buffered.") {
    REQUIRE(stream_recorder->FlushWithTimeout(
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            std::chrono::milliseconds{10})));
  }

  SECTION("Connections to satellites are reguarly reestablished.") {
    REQUIRE(IsEventuallyTrue([&] {
      tracer->StartSpan("abc");
      return mock_satellite->reports().size() > 1;
    }));
  }

  SECTION("Error responses from the satellite are logged.") {
    mock_satellite->SetRequestError();
    REQUIRE(IsEventuallyTrue([&] {
      tracer->StartSpan("abc");
      return logger_sink->contents().find("Error from satellite 404") !=
             std::string::npos;
    }));
  }

  SECTION("The recorder forces a reconnect if the satellite times out.") {
    mock_satellite->SetRequestTimeout();
    REQUIRE(IsEventuallyTrue([&] {
      tracer->StartSpan("abc");
      return logger_sink->contents().find(
                 "Failed to shutdown satellite connection gracefully") !=
             std::string::npos;
    }));
  }

  SECTION("The recorder reconnects if there's a premature close.") {
    mock_satellite->SetRequestPrematureClose();
    REQUIRE(IsEventuallyTrue([&] {
      tracer->StartSpan("abc");
      return logger_sink->contents().find(
                 "No status line from satellite response") != std::string::npos;
    }));
  }

  SECTION("Spans drop when the recorder's buffer fills.") {
    mock_satellite->SetThrottleReports();
    std::atomic<bool> stop{false};
    std::thread generator{GenerateSpans, std::ref(stop), std::ref(*tracer)};
    REQUIRE(IsEventuallyTrue([&] {
      return logger_sink->contents().find("Dropping span") != std::string::npos;
    }));
    stop = true;
    generator.join();
  }

  SECTION(
      "Flushes are incomplete when the satellite's connection is saturated.") {
    mock_satellite->SetThrottleReports();
    std::atomic<bool> stop{false};
    std::thread generator{GenerateSpans, std::ref(stop), std::ref(*tracer)};
    REQUIRE(IsEventuallyTrue([&] {
      return logger_sink->contents().find("Flushed partially") !=
             std::string::npos;
    }));
    stop = true;
    generator.join();
  }

  SECTION(
      "If a tracer is destroyed without shutting down first, it will close "
      "satellite sockets before the http sessions are finished") {
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
    tracer = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
  }

  SECTION("If a tracer is shut down, it waits for a response from the server") {
    tracer->StartSpan("abc");
    stream_recorder->FlushWithTimeout(std::chrono::milliseconds{50});
    stream_recorder->ShutdownWithTimeout(std::chrono::milliseconds{50});
    REQUIRE(logger_sink->contents().find("is readable") != std::string::npos);
  }

  SECTION("We can shut down a recorder twice") {
    tracer->StartSpan("abc");
    stream_recorder->FlushWithTimeout(std::chrono::milliseconds{50});
    stream_recorder->ShutdownWithTimeout(std::chrono::milliseconds{50});
    stream_recorder->ShutdownWithTimeout(std::chrono::milliseconds{50});
    REQUIRE(logger_sink->contents().find("is readable") != std::string::npos);
  }

  SECTION("Close will also shut down the recorder") {
    tracer->StartSpan("abc");
    tracer->Close();
    REQUIRE(logger_sink->contents().find("is readable") != std::string::npos);
  }

  SECTION("Shutdown will return early if it times out") {
    mock_satellite->SetRequestTimeout();
    tracer->StartSpan("abc");
    stream_recorder->FlushWithTimeout(std::chrono::milliseconds{50});
    stream_recorder->ShutdownWithTimeout(std::chrono::milliseconds{50});
    REQUIRE(logger_sink->contents().find("is readable") == std::string::npos);
  }
}
