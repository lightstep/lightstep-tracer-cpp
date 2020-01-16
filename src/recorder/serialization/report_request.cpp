#include "recorder/serialization/report_request.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
ReportRequest::ReportRequest(const std::shared_ptr<const std::string>& header,
                             std::unique_ptr<EmbeddedMetricsMessage>&& metrics)
    : header_{header} {
  (void)metrics;
}

//--------------------------------------------------------------------------------------------------
// AddSpan
//--------------------------------------------------------------------------------------------------
void ReportRequest::AddSpan(std::unique_ptr<SerializationChain>&& span) noexcept {
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

  (void)callback;
  (void)context;
  return true;
}
}  // namespace lightstep
