#include "common/hex_conversion.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// HexDigitLookupTable
//--------------------------------------------------------------------------------------------------
const unsigned char HexDigitLookupTable[513] =
    "000102030405060708090A0B0C0D0E0F"
    "101112131415161718191A1B1C1D1E1F"
    "202122232425262728292A2B2C2D2E2F"
    "303132333435363738393A3B3C3D3E3F"
    "404142434445464748494A4B4C4D4E4F"
    "505152535455565758595A5B5C5D5E5F"
    "606162636465666768696A6B6C6D6E6F"
    "707172737475767778797A7B7C7D7E7F"
    "808182838485868788898A8B8C8D8E8F"
    "909192939495969798999A9B9C9D9E9F"
    "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
    "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
    "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
    "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
    "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
    "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";

//------------------------------------------------------------------------------
// HexToUint64
//------------------------------------------------------------------------------
// Adopted from https://stackoverflow.com/a/11068850/4447365
opentracing::expected<uint64_t> HexToUint64(opentracing::string_view s) {
  static const unsigned char nil = std::numeric_limits<unsigned char>::max();
  static const std::array<unsigned char, 256> hextable = {
      {nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, 0,   1,   2,   3,   4,   5,   6,   7,
       8,   9,   nil, nil, nil, nil, nil, nil, nil, 10,  11,  12,  13,  14,
       15,  nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, 10,
       11,  12,  13,  14,  15,  nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil,
       nil, nil, nil, nil}};

  auto i = s.data();
  auto last = s.data() + s.size();

  // Remove any leading spaces
  while (i != last && static_cast<bool>(std::isspace(*i))) {
    ++i;
  }

  // Remove any trailing spaces
  while (i != last && static_cast<bool>(std::isspace(*(last - 1)))) {
    --last;
  }

  auto first = i;

  // Remove leading zeros
  while (i != last && *i == '0') {
    ++i;
  }

  auto length = std::distance(i, last);

  // Check for overflow
  if (length > 16) {
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

  uint64_t result = 0;
  for (; i != last; ++i) {
    auto value = hextable[*i];
    if (value == nil) {
      return opentracing::make_unexpected(
          std::make_error_code(std::errc::invalid_argument));
    }
    result = (result << 4) | value;
  }

  return result;
}
}  // namespace lightstep
