#include "recorder/manual_recorder.h"

#include <cassert>
#include <exception>

#include "common/report_request_framing.h"
#include "recorder/serialization/report_request.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// GetMetricsObserver
//--------------------------------------------------------------------------------------------------
static MetricsObserver& GetMetricsObserver(
    LightStepTracerOptions& tracer_options) {
  if (tracer_options.metrics_observer == nullptr) {
    tracer_options.metrics_observer.reset(new MetricsObserver{});
  }
  return *tracer_options.metrics_observer;
}

//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(Logger& logger, LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      tracer_options_{std::move(options)},
      transporter_{std::move(transporter)},
      metrics_{GetMetricsObserver(options)},
      span_buffer_{tracer_options_.max_buffered_spans.value()} {}

//--------------------------------------------------------------------------------------------------
// ReserveHeaderSpace
//--------------------------------------------------------------------------------------------------
Fragment ManualRecorder::ReserveHeaderSpace(ChainedStream& stream) {
  const size_t max_header_size = ReportRequestSpansMaxHeaderSize;
  static_assert(ChainedStream::BlockSize >= max_header_size,
                "BockSize too small");
  void* data;
  int size;
  if (!stream.Next(&data, &size)) {
    throw std::bad_alloc{};
  }
  stream.BackUp(size - static_cast<int>(max_header_size));
  return {data, static_cast<int>(max_header_size)};
}

//--------------------------------------------------------------------------------------------------
// RecordSpan
//--------------------------------------------------------------------------------------------------
void ManualRecorder::RecordSpan(
    Fragment header_fragment, std::unique_ptr<ChainedStream>&& span) noexcept {
  // Frame the Span
  char* header_data = static_cast<char*>(header_fragment.first);
  auto reserved_header_size = static_cast<size_t>(header_fragment.second);
  auto protobuf_body_size = span->ByteCount() - header_fragment.second;
  auto protobuf_header_size = WriteReportRequestSpansHeader(
      header_data, reserved_header_size, protobuf_body_size);
  span->CloseOutput();

  // Advance past reserved header space we didn't use.
  span->Seek(0, static_cast<int>(reserved_header_size - protobuf_header_size));

  if (!span_buffer_.Add(span)) {
    // Note: the compiler doesn't want to inline this logger call and it shows
    // up in profiling with high span droppage even if the logging isn't turned
    // on.
    //
    // Hence, the additional checking to avoid a function call.
    if (static_cast<int>(logger_.level()) <=
        static_cast<int>(LogLevel::debug)) {
      logger_.Debug("Dropping span");
    }
    metrics_.OnSpansDropped(1);
    span.reset();
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept try {
  return true;
} catch (const std::exception& e) {
  logger_.Error("Failed to flush report: ", e.what());
  return false;
}

//--------------------------------------------------------------------------------------------------
// PrepareForFork
//--------------------------------------------------------------------------------------------------
void ManualRecorder::PrepareForFork() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnForkedParent
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnForkedParent() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnForkedChild
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnForkedChild() noexcept {}

//--------------------------------------------------------------------------------------------------
// OnSuccess
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnSuccess(BufferChain& message) noexcept {
  auto report_request = dynamic_cast<ReportRequest*>(&message);
  assert(report_request != nullptr);
  if (report_request != nullptr) {
    // This should never happen
    return;
  }
  metrics_.OnSpansSent(report_request->num_spans());
}

//--------------------------------------------------------------------------------------------------
// OnFailure
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnFailure(BufferChain& message) noexcept {
  auto report_request = dynamic_cast<ReportRequest*>(&message);
  assert(report_request != nullptr);
  if (report_request != nullptr) {
    // This should never happen
    return;
  }
  metrics_.OnSpansDropped(report_request->num_spans());
  metrics_.UnconsumeDroppedSpans(report_request->num_dropped_spans());
}
}  // namespace lightstep
