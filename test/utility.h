#pragma once

#include <chrono>
#include <thread>

#include <opentracing/tracer.h>
#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
int LookupSpansDropped(const collector::ReportRequest& report);

bool HasTag(const collector::Span& span, opentracing::string_view key,
            const opentracing::Value& value);

bool HasRelationship(opentracing::SpanReferenceType relationship,
                     const collector::Span& span_a,
                     const collector::Span& span_b);

/**
 * Continually polls a provied functor until either it returns true or a timeout
 * is reached.
 * @param f supplies the functor to poll.
 * @param polling_interval supplies the amount of time between polls.
 * @param timeout supplies the timeout.
 * @return true if the functor returned true within the timeout; false,
 * otherwise.
 */
template <class F, class Duration1 = std::chrono::milliseconds,
          class Duration2 = std::chrono::minutes>
inline bool IsEventuallyTrue(
    F f, Duration1 polling_interval = std::chrono::milliseconds{50},
    Duration2 timeout = std::chrono::minutes{2}) {
  auto start = std::chrono::system_clock::now();
  while (true) {
    if (f()) {
      return true;
    }
    std::this_thread::sleep_for(polling_interval);
    if (std::chrono::system_clock::now() - start > timeout) {
      return false;
    }
  }
}
}  // namespace lightstep
