#include "common/utility.h"

#include <array>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include "common/hex_conversion.h"
#include "common/platform/time.h"

#include <opentracing/string_view.h>
#include <opentracing/value.h>

#include "lightstep-tracer-common/collector.pb.h"

namespace lightstep {
//------------------------------------------------------------------------------
// ToTimestamp
//------------------------------------------------------------------------------
google::protobuf::Timestamp ToTimestamp(
    const std::chrono::system_clock::time_point& t) {
  uint64_t seconds_since_epoch;
  uint32_t nano_fraction;
  std::tie(seconds_since_epoch, nano_fraction) = ProtobufFormatTimestamp(t);
  google::protobuf::Timestamp result;
  result.set_seconds(seconds_since_epoch);
  result.set_nanos(nano_fraction);
  return result;
}

timeval ToTimeval(std::chrono::microseconds microseconds) {
  timeval result;
  auto num_microseconds = microseconds.count();
  const size_t microseconds_in_second = 1000000;
  result.tv_sec = static_cast<decltype(result.tv_sec)>(num_microseconds /
                                                       microseconds_in_second);
  result.tv_usec = static_cast<decltype(result.tv_usec)>(
      num_microseconds % microseconds_in_second);
  return result;
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

std::string ToJson(const opentracing::Value& value) {
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

//--------------------------------------------------------------------------------------------------
// ToLower
//--------------------------------------------------------------------------------------------------
std::string ToLower(opentracing::string_view s) {
  std::string result;
  result.reserve(s.size());
  for (auto c : s) {
    result.push_back(static_cast<char>(std::tolower(c)));
  }
  return result;
}
}  // namespace lightstep
