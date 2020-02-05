#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <opentracing/string_view.h>
#include <opentracing/util.h>

namespace lightstep {
const size_t Num64BitHexDigits = std::numeric_limits<uint64_t>::digits / 4;

const size_t Num128BitHexDigits = 2 * Num64BitHexDigits;

const size_t Num32BitHexDigits = std::numeric_limits<uint32_t>::digits / 4;

const size_t Num8BitHexDigits = std::numeric_limits<uint8_t>::digits / 4;

extern const unsigned char HexDigitLookupTable[513];

/**
 * Writes a 64-bit number in hex.
 * @param x the number to write
 * @param output where to output the number
 * @return x as a hex string
 */
inline opentracing::string_view Uint64ToHex(uint64_t x, char* output) noexcept {
  for (int i = 8; i-- > 0;) {
    auto lookup_index = (x & 0xFF) * 2;
    output[i * 2] = HexDigitLookupTable[lookup_index];
    output[i * 2 + 1] = HexDigitLookupTable[lookup_index + 1];
    x >>= 8;
  }
  return {output, Num64BitHexDigits};
}

/**
 * Writes a 32-bit number in hex.
 * @param x the number to write
 * @param output where to output the number
 * @return x as a hex string
 */
inline opentracing::string_view Uint32ToHex(uint32_t x, char* output) noexcept {
  for (int i = 4; i-- > 0;) {
    auto lookup_index = (x & 0xFF) * 2;
    output[i * 2] = HexDigitLookupTable[lookup_index];
    output[i * 2 + 1] = HexDigitLookupTable[lookup_index + 1];
    x >>= 8;
  }
  return {output, Num32BitHexDigits};
}

/**
 * Writes a 8-bit number in hex.
 * @param x the number to write
 * @param output where to output the number
 * @return x as a hex string
 */
inline opentracing::string_view Uint8ToHex(uint8_t x, char* output) noexcept {
  auto lookup_index = static_cast<int>(x) * 2;
  output[0] = HexDigitLookupTable[lookup_index];
  output[1] = HexDigitLookupTable[lookup_index + 1];
  return {output, Num8BitHexDigits};
}

// Converts a hexidecimal number to a 64-bit integer. Either returns the number
// or an error code.
opentracing::expected<uint64_t> HexToUint64(
    opentracing::string_view s) noexcept;

opentracing::expected<void> HexToUint128(opentracing::string_view s,
                                         uint64_t& x_high,
                                         uint64_t& x_low) noexcept;

opentracing::expected<uint8_t> NormalizedHexToUint8(
    opentracing::string_view s) noexcept;

opentracing::expected<uint64_t> NormalizedHexToUint64(
    opentracing::string_view s) noexcept;

opentracing::expected<void> NormalizedHexToUint128(opentracing::string_view s,
                                                   uint64_t& x_high,
                                                   uint64_t& x_low) noexcept;

/**
 * Serialize integers to hex
 */
class HexSerializer {
 public:
  /**
   * Serialize a 64-bit integer in hex
   * @param x the number to serialize
   * @return a string_view of the hex encoding of x
   */
  opentracing::string_view Uint64ToHex(uint64_t x) noexcept {
    return lightstep::Uint64ToHex(x, buffer_.data());
  }

  /**
   * Serialize a 128-bit integer in hex
   * @param x_high the high part of the number to serialize
   * @param x_low the low part of the number to serialize
   * @return a string_view of the hex encoding of x_high,x_low
   */
  opentracing::string_view Uint128ToHex(uint64_t x_high,
                                        uint64_t x_low) noexcept {
    if (x_high == 0) {
      return this->Uint64ToHex(x_low);
    }
    lightstep::Uint64ToHex(x_high, buffer_.data());
    lightstep::Uint64ToHex(x_low, buffer_.data() + Num64BitHexDigits);
    return opentracing::string_view{buffer_.data(), buffer_.size()};
  }

 private:
  std::array<char, Num64BitHexDigits * 2> buffer_;
};
}  // namespace lightstep
