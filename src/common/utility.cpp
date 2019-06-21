#include "common/utility.h"

#include <opentracing/string_view.h>
#include <opentracing/value.h>

#include "lightstep-tracer-common/collector.pb.h"

//#include <unistd.h>
#include <array>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <limits>

//#include <pthread.h>

/*
stuff we need to replace:
 - suseconds_t
 - readlink
 - ssize_t
*/

namespace lightstep {
//------------------------------------------------------------------------------
// ToTimestamp
//------------------------------------------------------------------------------
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t) {
  using namespace std::chrono;
  auto nanos = duration_cast<nanoseconds>(t.time_since_epoch()).count();
  google::protobuf::Timestamp ts;
  const uint64_t nanosPerSec = 1000000000;
  ts.set_seconds(nanos / nanosPerSec);
  ts.set_nanos(nanos % nanosPerSec);
  return ts;
}

timeval ToTimeval(std::chrono::microseconds microseconds) {
  timeval result;
  auto num_microseconds = microseconds.count();
  const size_t microseconds_in_second = 1000000;
  result.tv_sec =
      static_cast<time_t>(num_microseconds / microseconds_in_second);
  result.tv_usec =
      static_cast<long>(num_microseconds % microseconds_in_second);
  return result;
}

//------------------------------------------------------------------------------
// GetProgramName
//------------------------------------------------------------------------------
std::string GetProgramName() {
  const int path_max = 1024;
  TCHAR exe_path_char[path_max]; 

  // this returns a DWORD by default
  int size = (int) GetModuleFileName(NULL, exe_path_char, path_max);

  std::string exe_path(exe_path_char);

  if (size == 0) {
    return "c++-program";  // Dunno...
  }


  size_t lslash = exe_path.rfind('/');
  if (lslash != std::string::npos) {
    return exe_path.substr(lslash + 1);
  }
  return exe_path;
}

//------------------------------------------------------------------------------
// WriteEscapedString
//------------------------------------------------------------------------------
// The implementation is based off of this answer from StackOverflow:
// https://stackoverflow.com/a/33799784
static void WriteEscapedString(std::ostringstream& writer,
                               opentracing::string_view s) {
  writer << '"';
  for (char c : s) {
    switch (c) {
      case '"':
        writer << R"(\")";
        break;
      case '\\':
        writer << R"(\\)";
        break;
      case '\b':
        writer << R"(\b)";
        break;
      case '\n':
        writer << R"(\n)";
        break;
      case '\r':
        writer << R"(\r)";
        break;
      case '\t':
        writer << R"(\t)";
        break;
      default:
        if ('\x00' <= c && c <= '\x1f') {
          writer << R"(\u)";
          writer << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<int>(c);
        } else {
          writer << c;
        }
    }
  }
  writer << '"';
}

//------------------------------------------------------------------------------
// ToJson
//------------------------------------------------------------------------------
static void ToJson(std::ostringstream& writer, const opentracing::Value& value);

namespace {
struct JsonValueVisitor {
  std::ostringstream& writer;

  void operator()(bool value) {
    if (value) {
      writer << "true";
    } else {
      writer << "false";
    }
  }

  void operator()(double value) {
    if (std::isnan(value)) {
      writer << R"("NaN")";
    } else if (std::isinf(value)) {
      if (std::signbit(value)) {
        writer << R"("-Inf")";
      } else {
        writer << R"("+Inf")";
      }
    } else {
      writer << value;
    }
  }

  void operator()(int64_t value) { writer << value; }

  void operator()(uint64_t value) { writer << value; }

  void operator()(const std::string& s) { WriteEscapedString(writer, s); }

  void operator()(std::nullptr_t) { writer << "null"; }

  void operator()(const char* s) { WriteEscapedString(writer, s); }

  void operator()(const opentracing::Values& values) {
    writer << '[';
    size_t i = 0;
    for (const auto& value : values) {
      ToJson(writer, value);
      if (++i < values.size()) {
        writer << ',';
      }
    }
    writer << ']';
  }

  void operator()(const opentracing::Dictionary& dictionary) {
    writer << '{';
    size_t i = 0;
    for (const auto& key_value : dictionary) {
      WriteEscapedString(writer, key_value.first);
      writer << ':';
      ToJson(writer, key_value.second);
      if (++i < dictionary.size()) {
        writer << ',';
      }
    }
    writer << '}';
  }
};
}  // anonymous namespace

static void ToJson(std::ostringstream& writer,
                   const opentracing::Value& value) {
  JsonValueVisitor value_visitor{writer};
  apply_visitor(value_visitor, value);
}

static std::string ToJson(const opentracing::Value& value) {
  std::ostringstream writer;
  writer.exceptions(std::ios::badbit | std::ios::failbit);
  ToJson(writer, value);
  return writer.str();
}

//------------------------------------------------------------------------------
// ToKeyValue
//------------------------------------------------------------------------------
namespace {
struct ValueVisitor {
  collector::KeyValue& key_value;
  const opentracing::Value& original_value;

  void operator()(bool value) const { key_value.set_bool_value(value); }

  void operator()(double value) const { key_value.set_double_value(value); }

  void operator()(int64_t value) const { key_value.set_int_value(value); }

  void operator()(uint64_t value) const {
    // There's no uint64_t value type so cast to an int64_t.
    key_value.set_int_value(static_cast<int64_t>(value));
  }

  void operator()(const std::string& s) const { key_value.set_string_value(s); }

  void operator()(std::nullptr_t) const { key_value.set_bool_value(false); }

  void operator()(const char* s) const { key_value.set_string_value(s); }

  void operator()(const opentracing::Values& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }

  void operator()(const opentracing::Dictionary& /*unused*/) const {
    key_value.set_json_value(ToJson(original_value));
  }
};
}  // anonymous namespace

collector::KeyValue ToKeyValue(opentracing::string_view key,
                               const opentracing::Value& value) {
  collector::KeyValue key_value;
  key_value.set_key(key);
  ValueVisitor value_visitor{key_value, value};
  apply_visitor(value_visitor, value);
  return key_value;
}

//------------------------------------------------------------------------------
// LogReportResponse
//------------------------------------------------------------------------------
void LogReportResponse(Logger& logger, bool verbose,
                       const collector::ReportResponse& response) {
  for (auto& message : response.errors()) {
    logger.Error(message);
  }
  for (auto& message : response.warnings()) {
    logger.Warn(message);
  }
  if (verbose) {
    logger.Info(R"(Report: resp=")", response.ShortDebugString(), R"(")");
    for (auto& message : response.infos()) {
      logger.Info(message);
    }
  }
}

//------------------------------------------------------------------------------
// Uint64ToHex
//------------------------------------------------------------------------------
// This uses the lookup table solution described on this blog post
// https://johnnylee-sde.github.io/Fast-unsigned-integer-to-hex-string/
opentracing::string_view Uint64ToHex(uint64_t x, char* output) {
  static const unsigned char digits[513] =
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
  for (int i = 8; i-- > 0;) {
    auto lookup_index = (x & 0xFF) * 2;
    output[i * 2] = digits[lookup_index];
    output[i * 2 + 1] = digits[lookup_index + 1];
    x >>= 8;
  }
  return {output, Num64BitHexDigits};
}

//------------------------------------------------------------------------------
// HexToUint64
//------------------------------------------------------------------------------
// Adopted from https://stackoverflow.com/a/11068850/4447365
opentracing::expected<uint64_t> HexToUint64(opentracing::string_view s) {
// get rid of windows max() macro so we can use [namespace]::max()
#undef max 


  std::cout << std::numeric_limits<float>::max() << std::endl;

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

//--------------------------------------------------------------------------------------------------
// ReadChunkHeader
//--------------------------------------------------------------------------------------------------
bool ReadChunkHeader(google::protobuf::io::ZeroCopyInputStream& stream,
                     size_t& chunk_size) {
  std::array<char, Num64BitHexDigits> digits;
  int digit_index = 0;
  const char* data;
  int size;
  while (stream.Next(reinterpret_cast<const void**>(&data), &size)) {
    for (int i = 0; i < size; ++i) {
      if (data[i] == '\r') {
        auto chunk_size_maybe =
            HexToUint64({digits.data(), static_cast<size_t>(digit_index)});
        if (!chunk_size_maybe) {
          return false;
        }
        stream.BackUp(size - i);
        if (!stream.Skip(2)) {
          return false;
        }
        chunk_size = *chunk_size_maybe;
        return true;
      }
      if (digit_index >= static_cast<int>(digits.size())) {
        return false;
      }
      digits[digit_index++] = data[i];
    }
  }
  return false;
}
}  // namespace lightstep
