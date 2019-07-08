#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include "common/circular_buffer2.h"
#include "common/serialization_chain.h"
#include "recorder/stream_recorder/connection_stream2.h"

#include <opentracing/string_view.h>

namespace lightstep {
/**
 * Randomly writes binary numbers into a given circular buffer.
 * @param buffer the buffer to write numbers into.
 * @param numbers outputs the numbers written.
 * @param num_threads the number of threads to write numbers on.
 * @param n the number of numbers to write.
 */
void RunBinaryNumberProducer(CircularBuffer2<SerializationChain>& buffer,
                             std::vector<uint32_t>& numbers, size_t num_threads,
                             size_t n);

/**
 * Reads numbers out of the given ChunkCircularBuffer through the given
 * ConnectionStreams.
 * @param span_stream the SpanStream connected the ChunkCircularBuffer.
 * @param connection_streams the ConnectionStreams to read numbers out of.
 * @param exit indicates that the producer has finished.
 * @param numbers outputs the numbers read.
 */
void RunBinaryNumberConnectionConsumer(
    SpanStream2& span_stream,
    std::vector<ConnectionStream2>& connection_streams, std::atomic<bool>& exit,
    std::vector<uint32_t>& numbers);
}  // namespace lightstep
