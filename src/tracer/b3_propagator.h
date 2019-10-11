#pragma once

#include <opentracing/string_view.h>

namespace lightstep {
class B3Propagator {
 public:
   opentracing::string_view trace_id_key() const noexcept;

   opentracing::string_view span_id_key() const noexcept;

   opentracing::string_view sampled_key() const noexcept;

   opentracing::string_view baggage_prefix() const noexcept { return {}; }

   bool supports_baggage() const noexcept { return false; }
};
} // namespace lightstep
