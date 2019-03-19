#pragma once

#include <chrono>
#include <thread>

#include "common/chunk_circular_buffer.h"
#include "network/fragment_input_stream.h"

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
 * Determines whether we can connect to a given localhost port.
 * @param port supplies the port to test for connectivity.
 * @return true if we can connect to localhost:port.
 */
bool CanConnect(uint16_t port) noexcept;

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

/**
 * Builds a string concatenating all the pieces of a fragment set.
 * @param fragment_input_stream supplies the fragments to concatenate.
 * @return a string concatenating all the fragments.
 */
std::string ToString(const FragmentInputStream& fragment_input_stream);

bool AddString(ChunkCircularBuffer& buffer, opentracing::string_view s);
}  // namespace lightstep
