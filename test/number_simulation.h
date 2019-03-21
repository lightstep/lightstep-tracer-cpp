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
std::tuple<uint32_t, opentracing::string_view> GenerateRandomBinaryNumber(
    size_t max_digits);

uint32_t ReadBinaryNumber(google::protobuf::io::ZeroCopyInputStream& stream,
                          size_t num_digits);

void RunBinaryNumberProducer(ChunkCircularBuffer& buffer,
                             std::vector<uint32_t>& numbers, size_t num_threads,
                             size_t n);

void RunBinaryNumberConsumer(ChunkCircularBuffer& buffer,
                             std::atomic<bool>& exit,
                             std::vector<uint32_t>& numbers);

void RunBinaryNumberConnectionConsumer(
    ChunkCircularBuffer& buffer,
    std::vector<ConnectionStream>& connection_streams,
    std::atomic<bool>& exit,
    std::vector<uint32_t>& numbers);
}  // namespace lightstep
