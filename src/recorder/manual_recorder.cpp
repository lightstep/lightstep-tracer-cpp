#include "recorder/manual_recorder.h"

#include <cassert>
#include <exception>

#include "common/random.h"
#include "common/report_request_framing.h"
#include "recorder/serialization/report_request.h"
#include "recorder/serialization/report_request_header.h"

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
      report_request_header_{new std::string{
          WriteReportRequestHeader(tracer_options_, GenerateId())}},
      metrics_{GetMetricsObserver(tracer_options_)},
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
  auto header_data = static_cast<char*>(header_fragment.first);
  auto reserved_header_size = static_cast<size_t>(header_fragment.second);
  auto protobuf_body_size = span->ByteCount() - header_fragment.second;
  auto protobuf_header_size = WriteReportRequestSpansHeader(
      header_data, reserved_header_size, protobuf_body_size);
  span->CloseOutput();

  // Advance past reserved header space we didn't use.
  span->Seek(0, static_cast<int>(reserved_header_size - protobuf_header_size));

  auto was_added = span_buffer_.Add(span);
  if (was_added) {
    return;
  }
  transporter_->OnSpanBufferFull();

  // Attempt to add the span again in case the transporter decided to flush
  if (!span_buffer_.Add(span)) {
    metrics_.OnSpansDropped(1);
  }
}

//--------------------------------------------------------------------------------------------------
// FlushWithTimeout
//--------------------------------------------------------------------------------------------------
bool ManualRecorder::FlushWithTimeout(
    std::chrono::system_clock::duration /*timeout*/) noexcept try {
  std::unique_ptr<ReportRequest> report_request{new ReportRequest{
      report_request_header_, metrics_.ConsumeDroppedSpans()}};
  {
    std::lock_guard<std::mutex> lock_guard{flush_mutex_};
    auto num_spans = span_buffer_.size();
    if (num_spans == 0 && report_request->num_dropped_spans() == 0) {
      // Nothing to do
      return true;
    }
    span_buffer_.Consume(
        num_spans, [&](CircularBufferRange<AtomicUniquePtr<ChainedStream>> &
                       spans) noexcept {
          spans.ForEach([&](AtomicUniquePtr<ChainedStream> & span) noexcept {
            std::unique_ptr<ChainedStream> span_out;
            span.Swap(span_out);
            report_request->AddSpan(std::move(span_out));
            return true;
          });
          return true;
        });
  }
  transporter_->Send(std::unique_ptr<BufferChain>{report_request.release()},
                     *this);
  metrics_.OnFlush();
  return true;
} catch (const std::exception& e) {
  logger_.Error("Failed to flush report: ", e.what());
  return false;
}

//--------------------------------------------------------------------------------------------------
// OnForkedChild
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnForkedChild() noexcept {
  metrics_.ConsumeDroppedSpans();
  span_buffer_.Clear();
}

//--------------------------------------------------------------------------------------------------
// OnSuccess
//--------------------------------------------------------------------------------------------------
void ManualRecorder::OnSuccess(BufferChain& message) noexcept {
  auto report_request = dynamic_cast<ReportRequest*>(&message);
  assert(report_request != nullptr);
  if (report_request == nullptr) {
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
  if (report_request == nullptr) {
    // This should never happen
    return;
  }
  metrics_.OnSpansDropped(report_request->num_spans());
  metrics_.UnconsumeDroppedSpans(report_request->num_dropped_spans());
}
}  // namespace lightstep
