#pragma once

#include <limits>

#include "common/serialization.h"

namespace lightstep {
constexpr size_t ReportRequestSpansField = 3;

constexpr size_t ReportRequestSpansMaxHeaderSize =
    StaticKeySerializationSize<ReportRequestSpansField,
                               WireType::LengthDelimited>::value +
    google::protobuf::io::CodedOutputStream::StaticVarintSize32<
        std::numeric_limits<uint32_t>::max()>::value;

/**
 * Write framing for a span in a collector::ReportRequest message.
 * @param data start of the buffer to write to.
 * @param size the size of the buffer
 * @param chunk_size the size of the chunk.
 *
 * Note: Data is written from the end of the provided buffer.
 */
size_t WriteReportRequestSpansHeader(char* data, size_t size,
                                     uint32_t body_size) noexcept;
}  // namespace lightstep
