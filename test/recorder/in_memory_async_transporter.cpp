#include "test/recorder/in_memory_async_transporter.h"

#include <algorithm>
#include <iterator>
#include <string>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
InMemoryAsyncTransporter::InMemoryAsyncTransporter(
    std::function<void()> on_span_buffer_full)
    : on_span_buffer_full_{std::move(on_span_buffer_full)} {}

//--------------------------------------------------------------------------------------------------
// Succeed
//--------------------------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Succeed() noexcept {
  if (active_callback_ == nullptr || active_message_ == nullptr) {
    std::terminate();
  }
  std::string s(active_message_->num_bytes(), ' ');
  active_message_->CopyOut(&s[0], s.size());
  collector::ReportRequest report;
  if (!report.ParseFromString(s)) {
    std::terminate();
  }
  reports_.emplace_back(report);
  std::copy(report.spans().begin(), report.spans().end(),
            std::back_inserter(spans_));

  active_callback_->OnSuccess(*active_message_);
  active_callback_ = nullptr;
  active_message_.reset();
}

//--------------------------------------------------------------------------------------------------
// Fail
//--------------------------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Fail() noexcept {
  if (active_callback_ == nullptr || active_message_ == nullptr) {
    std::terminate();
  }
  active_callback_->OnFailure(*active_message_);
  active_callback_ = nullptr;
  active_message_.reset();
}

//--------------------------------------------------------------------------------------------------
// OnSpanBufferFull
//--------------------------------------------------------------------------------------------------
void InMemoryAsyncTransporter::OnSpanBufferFull() noexcept {
  if (on_span_buffer_full_) {
    on_span_buffer_full_();
  }
}

//--------------------------------------------------------------------------------------------------
// Send
//--------------------------------------------------------------------------------------------------
void InMemoryAsyncTransporter::Send(std::unique_ptr<BufferChain>&& message,
                                    Callback& callback) noexcept {
  active_callback_ = &callback;
  active_message_ = std::move(message);
}
}  // namespace lightstep
