#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include "common/chunk_circular_buffer.h"
#include "recorder/stream_recorder/connection_stream.h"

#include <google/protobuf/io/zero_copy_stream.h>
#include <opentracing/string_view.h>

namespace lightstep {
void RunBinaryNumberProducer(ChunkCircularBuffer& buffer,
                             std::vector<uint32_t>& numbers, size_t num_threads,
                             size_t n);

void RunBinaryNumberConsumer(ChunkCircularBuffer& buffer,
                             std::atomic<bool>& exit,
                             std::vector<uint32_t>& numbers);

void RunBinaryNumberConnectionConsumer(
    SpanStream& span_stream, std::vector<ConnectionStream>& connection_streams,
    std::atomic<bool>& exit, std::vector<uint32_t>& numbers);
}  // namespace lightstep
