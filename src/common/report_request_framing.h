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

size_t WriteReportRequestSpansHeader(char* data, size_t size,
                                     uint32_t body_size) noexcept;
}  // namespace lightstep
