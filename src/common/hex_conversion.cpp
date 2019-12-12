#include "common/hex_conversion.h"

#include <cassert>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// Nil
//--------------------------------------------------------------------------------------------------
static const unsigned char Nil = std::numeric_limits<unsigned char>::max();

//--------------------------------------------------------------------------------------------------
// HexTable
//--------------------------------------------------------------------------------------------------
static const std::array<unsigned char, 256> HexTable = {
    {Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, 10,  11,  12,  13,  14,  15,  Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, 10,  11,  12,  13,  14,  15,  Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil, Nil,
     Nil}};

//--------------------------------------------------------------------------------------------------
// HexToUintImpl
//--------------------------------------------------------------------------------------------------
template <class T>
static opentracing::expected<T> HexToUintImpl(const char* i,
                                              const char* last) noexcept {
  T result = 0;
  for (; i != last; ++i) {
    auto value = HexTable[*i];
    if (value == Nil) {
      return opentracing::make_unexpected(
          std::make_error_code(std::errc::invalid_argument));
    }
    result = (result << 4) | value;
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// RemoveSurroundingWhitespace
//--------------------------------------------------------------------------------------------------
static void RemoveSurroundingWhitespace(const char*& i,
                                        const char*& last) noexcept {
  // Remove any leading spaces
  while (i != last && static_cast<bool>(std::isspace(*i))) {
    ++i;
  }

  // Remove any trailing spaces
  while (i != last && static_cast<bool>(std::isspace(*(last - 1)))) {
    --last;
  }
}

//--------------------------------------------------------------------------------------------------
// HexDigitLookupTable
//--------------------------------------------------------------------------------------------------
const unsigned char HexDigitLookupTable[513] =
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

//------------------------------------------------------------------------------
// HexToUint64
//------------------------------------------------------------------------------
// Adopted from https://stackoverflow.com/a/11068850/4447365
opentracing::expected<uint64_t> HexToUint64(
    opentracing::string_view s) noexcept {
  auto i = s.data();
  auto last = s.data() + s.size();

  RemoveSurroundingWhitespace(i, last);

  auto first = i;

  // Remove leading zeros
  while (i != last && *i == '0') {
    ++i;
  }

  auto length = static_cast<size_t>(std::distance(i, last));

  // Check for overflow
  if (length > Num64BitHexDigits) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::value_too_large));
  }

  // Check for an empty string
  if (length == 0) {
    // Handle the case of the string being all zeros
    if (first != i) {
      return 0;
    }
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }

  return HexToUintImpl<uint64_t>(i, last);
}

//--------------------------------------------------------------------------------------------------
// HexToUint128
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> HexToUint128(opentracing::string_view s,
                                         uint64_t& x_high,
                                         uint64_t& x_low) noexcept {
  auto i = s.data();
  auto last = s.data() + s.size();

  RemoveSurroundingWhitespace(i, last);

  auto first = i;

  // Remove leading zeros
  while (i != last && *i == '0') {
    ++i;
  }

  auto length = static_cast<size_t>(std::distance(i, last));

  // Check for overflow
  if (length > 2 * Num64BitHexDigits) {
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::value_too_large));
  }

  // Check for an empty string
  if (length == 0) {
    // Handle the case of the string being all zeros
    if (first != i) {
      x_high = 0;
      x_low = 0;
      return {};
    }
    return opentracing::make_unexpected(
        std::make_error_code(std::errc::invalid_argument));
  }

  // handle the case when we have a number that fits in a 64-bit integer
  if (length <= Num64BitHexDigits) {
    x_high = 0;
    auto x_maybe = HexToUintImpl<uint64_t>(i, last);
    if (!x_maybe) {
      return opentracing::make_unexpected(x_maybe.error());
    }
    x_low = *x_maybe;
    return {};
  }

  // handle the case when we have a number that requires more than a single
  // 64-bit integer
  auto boundary = i + (length - Num64BitHexDigits);
  auto x_high_maybe = HexToUintImpl<uint64_t>(i, boundary);
  if (!x_high_maybe) {
    return opentracing::make_unexpected(x_high_maybe.error());
  }
  x_high = *x_high_maybe;

  auto x_low_maybe = HexToUintImpl<uint64_t>(boundary, last);
  if (!x_low_maybe) {
    return opentracing::make_unexpected(x_low_maybe.error());
  }
  x_low = *x_low_maybe;
  return {};
}

//--------------------------------------------------------------------------------------------------
// NormalizedHexToUint8
//--------------------------------------------------------------------------------------------------
opentracing::expected<uint8_t> NormalizedHexToUint8(
    opentracing::string_view s) noexcept {
  assert(s.size() == Num8BitHexDigits);
  return HexToUintImpl<uint8_t>(s.data(), s.data() + s.size());
}

//--------------------------------------------------------------------------------------------------
// NormalizedHexToUint64
//--------------------------------------------------------------------------------------------------
opentracing::expected<uint64_t> NormalizedHexToUint64(
    opentracing::string_view s) noexcept {
  assert(s.size() == Num64BitHexDigits);
  return HexToUintImpl<uint64_t>(s.data(), s.data() + s.size());
}

//--------------------------------------------------------------------------------------------------
// NormalizedHexToUint128
//--------------------------------------------------------------------------------------------------
opentracing::expected<void> NormalizedHexToUint128(opentracing::string_view s,
                                                   uint64_t& x_high,
                                                   uint64_t& x_low) noexcept {
  assert(s.size() == 2 * Num64BitHexDigits);
  auto x_maybe =
      HexToUintImpl<uint64_t>(s.data(), s.data() + Num64BitHexDigits);
  if (!x_maybe) {
    return opentracing::make_unexpected(x_maybe.error());
  }
  x_high = *x_maybe;
  x_maybe = HexToUintImpl<uint64_t>(s.data() + Num64BitHexDigits,
                                    s.data() + 2 * Num64BitHexDigits);
  if (!x_maybe) {
    return opentracing::make_unexpected(x_maybe.error());
  }
  x_low = *x_maybe;
  return {};
}
}  // namespace lightstep
