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
// num_fragments
//--------------------------------------------------------------------------------------------------
size_t ReportRequest::num_fragments() const noexcept { return 0; }

//--------------------------------------------------------------------------------------------------
// num_bytes
//--------------------------------------------------------------------------------------------------
size_t ReportRequest::num_bytes() const noexcept { return 0; }

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
