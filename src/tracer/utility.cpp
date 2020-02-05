#include "tracer/utility.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// AppendTraceState
//--------------------------------------------------------------------------------------------------
void AppendTraceState(std::string& trace_state,
                      opentracing::string_view key_values) {
  if (trace_state.empty()) {
    trace_state.assign(key_values.data(), key_values.size());
    return;
  }
  trace_state.reserve(trace_state.size() + 1 + key_values.size());
  trace_state.append(",");
  trace_state.append(key_values.data(), key_values.size());
}
}  // namespace lightstep
