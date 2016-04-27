#include "span.h"
#include "tracer.h"

namespace lightstep {

Span StartSpan(const std::string& operation_name) {
  return Tracer::Global().StartSpan(operation_name);
}

Span StartChildSpan(Span parent, const std::string& operation_name) {
  return Tracer::Global().StartSpanWithOptions(StartSpanOptions().
					       SetOperationName(operation_name).
					       SetParent(parent));
}

} // namespace lightstep
