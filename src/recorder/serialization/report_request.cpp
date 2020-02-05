#include "recorder/serialization/report_request.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ReportRequest::ReportRequest(std::shared_ptr<const std::string> header,
                             int num_dropped_spans)
    : header_{std::move(header)} {
  num_bytes_ = static_cast<int>(header_->size());
  if (num_dropped_spans == 0) {
    return;
  }
  auto metrics = new EmbeddedMetricsMessage{};
  metrics_.reset(metrics);
  metrics->set_num_dropped_spans(num_dropped_spans);
  metrics_fragment_ = metrics->MakeFragment();
  num_bytes_ += metrics_fragment_.second;
}

//--------------------------------------------------------------------------------------------------
// AddSpan
//--------------------------------------------------------------------------------------------------
void ReportRequest::AddSpan(std::unique_ptr<ChainedStream>&& span) noexcept {
  ++num_spans_;
  num_fragments_ += span->num_fragments();
  span->ForEachFragment([&](void* /*data*/, int length) {
    num_bytes_ += static_cast<size_t>(length);
    return true;
  });
  if (spans_ == nullptr) {
    spans_ = std::move(span);
    return;
  }
  spans_->Append(std::move(span));
}

//--------------------------------------------------------------------------------------------------
// num_dropped_spans
//--------------------------------------------------------------------------------------------------
int ReportRequest::num_dropped_spans() const noexcept {
  if (metrics_ == nullptr) {
    return 0;
  }
  return metrics_->num_dropped_spans();
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
  return spans_->ForEachFragment([&](void* data, int length) {
    return callback(context, data, static_cast<size_t>(length));
  });
}
}  // namespace lightstep
