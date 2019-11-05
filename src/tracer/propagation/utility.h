#pragma once

#include "common/utility.h"

#include <opentracing/propagation.h>
#include <opentracing/string_view.h>

namespace lightstep {
template <class KeyCompare>
opentracing::expected<opentracing::string_view> LookupKey(
    const opentracing::TextMapReader& carrier, opentracing::string_view key,
    KeyCompare key_compare) {
  // First try carrier.LookupKey since that can potentially be the fastest
  // approach.
  auto result = carrier.LookupKey(key);
  if (result || !AreErrorsEqual(result.error(),
                                opentracing::lookup_key_not_supported_error)) {
    return result;
  }

  // Fall back to iterating through all of the keys.
  result = opentracing::make_unexpected(opentracing::key_not_found_error);
  auto was_successful = carrier.ForeachKey(
      [&](opentracing::string_view carrier_key,
          opentracing::string_view value) -> opentracing::expected<void> {
        if (!key_compare(carrier_key, key)) {
          return {};
        }
        result = value;

        // Found key, so bail out of the loop with a success error code.
        return opentracing::make_unexpected(std::error_code{});
      });
  if (!was_successful && was_successful.error() != std::error_code{}) {
    return opentracing::make_unexpected(was_successful.error());
  }
  return result;
}
}  // namespace lightstep
