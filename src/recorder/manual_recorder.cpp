#include "recorder/manual_recorder.h"

#include "common/report_request_framing.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ManualRecorder::ManualRecorder(Logger& logger, LightStepTracerOptions options,
                               std::unique_ptr<AsyncTransporter>&& transporter)
    : logger_{logger},
      tracer_options_{std::move(options)},
      transporter_{std::move(transporter)},
      span_buffer_{tracer_options_.max_buffered_spans.value()} {
  // If no MetricsObserver was provided, use a default one that does nothing.
  if (tracer_options_.metrics_observer == nullptr) {
    tracer_options_.metrics_observer.reset(new MetricsObserver{});
  }
}

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
    /* metrics_.OnSpansDropped(1); */
    span.reset();
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept {
  return true;
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
}  // namespace lightstep
