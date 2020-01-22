#include "recorder/serialization/report_request.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ReportRequest::ReportRequest(const std::shared_ptr<const std::string>& header,
                             int num_dropped_spans)
    : header_{header} {
  (void)num_dropped_spans;
  num_bytes_ = static_cast<int>(header_->size());
}

//--------------------------------------------------------------------------------------------------
// AddSpan
//--------------------------------------------------------------------------------------------------
void ReportRequest::AddSpan(
    std::unique_ptr<SerializationChain>&& span) noexcept {
  ++num_spans_;
  num_fragments_ += span->num_fragments();
  num_bytes_ += span->num_bytes_after_framing();
  if (spans_ == nullptr) {
    spans_ = std::move(span);
    return;
  }
  spans_->Chain(std::move(span));
}

//--------------------------------------------------------------------------------------------------
// ForEachFragment
//--------------------------------------------------------------------------------------------------
bool ReportRequest::ForEachFragment(FragmentCallback callback,
                                    void* context) const {
  if (!callback(context, static_cast<const void*>(header_->data()),
                header_->size())) {
    return false;
  }
  if (metrics_ != nullptr) {
    if (!callback(context, metrics_fragment_.first,
                  static_cast<size_t>(metrics_fragment_.second))) {
      return false;
    }
  }
  if (spans_ == nullptr) {
    return true;
  }
  return spans_->ForEachSerializationChain(
      [&](const SerializationChain& chain) {
        return chain.ForEachFragment([&](void* data, int length) {
          return callback(context, data, static_cast<size_t>(length));
        });
      });
}
}  // namespace lightstep
