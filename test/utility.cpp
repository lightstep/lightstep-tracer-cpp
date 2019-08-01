#include "test/utility.h"

#include <algorithm>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "common/serialization.h"
#include "common/serialization_chain.h"
#include "common/utility.h"
#include "network/socket.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/message_differencer.h>

namespace lightstep {
//------------------------------------------------------------------------------
// LookupSpansDropped
//------------------------------------------------------------------------------
int LookupSpansDropped(const collector::ReportRequest& report) {
  if (!report.has_internal_metrics()) {
    return 0;
  }
  auto& counts = report.internal_metrics().counts();
  auto iter = std::find_if(counts.begin(), counts.end(),
                           [](const collector::MetricsSample& sample) {
                             return sample.name() == "spans.dropped";
                           });
  if (iter == counts.end()) {
    return 0;
  }
  if (iter->value_case() != collector::MetricsSample::kIntValue) {
    std::cerr << "spans.dropped not of type int\n";
    std::terminate();
  }
  return static_cast<int>(iter->int_value());
}

//------------------------------------------------------------------------------
// HasTag
//------------------------------------------------------------------------------
bool HasTag(const collector::Span& span, opentracing::string_view key,
            const opentracing::Value& value) {
  auto key_value = ToKeyValue(key, value);
  return std::any_of(
      std::begin(span.tags()), std::end(span.tags()),
      [&](const collector::KeyValue& other) {
        return google::protobuf::util::MessageDifferencer::Equals(key_value,
                                                                  other);
      });
}

//------------------------------------------------------------------------------
// HasRelationship
//------------------------------------------------------------------------------
bool HasRelationship(opentracing::SpanReferenceType relationship,
                     const collector::Span& span_a,
                     const collector::Span& span_b) {
  collector::Reference reference;
  switch (relationship) {
    case opentracing::SpanReferenceType::ChildOfRef:
      reference.set_relationship(collector::Reference::CHILD_OF);
      break;
    case opentracing::SpanReferenceType::FollowsFromRef:
      reference.set_relationship(collector::Reference::FOLLOWS_FROM);
      break;
  }
  *reference.mutable_span_context() = span_b.span_context();
  return std::any_of(
      std::begin(span_a.references()), std::end(span_a.references()),
      [&](const collector::Reference& other) {
        return google::protobuf::util::MessageDifferencer::Equals(reference,
                                                                  other);
      });
}

//--------------------------------------------------------------------------------------------------
// CanConnect
//--------------------------------------------------------------------------------------------------
bool CanConnect(uint16_t port) noexcept try {
  Socket socket{};
  socket.SetReuseAddress();
  IpAddress ip_address{"127.0.0.1", port};
  return socket.Connect(ip_address.addr(), sizeof(ip_address.ipv4_address())) ==
         0;
} catch (...) {
  return false;
}

//--------------------------------------------------------------------------------------------------
// ToString
//--------------------------------------------------------------------------------------------------
std::string ToString(const FragmentInputStream& fragment_input_stream) {
  std::string result;
  fragment_input_stream.ForEachFragment([&result](void* data, int size) {
    result.append(static_cast<char*>(data), static_cast<size_t>(size));
    return true;
  });
  return result;
}

//--------------------------------------------------------------------------------------------------
// AddString
//--------------------------------------------------------------------------------------------------
bool AddString(CircularBuffer<SerializationChain>& buffer,
               const std::string& s) {
  std::unique_ptr<SerializationChain> chain{new SerializationChain{}};
  {
    google::protobuf::io::CodedOutputStream stream{chain.get()};
    stream.WriteString(s);
  }
  chain->AddFraming();
  return buffer.Add(chain);
}

//--------------------------------------------------------------------------------------------------
// AddSpanChunkFraming
//--------------------------------------------------------------------------------------------------
std::string AddSpanChunkFraming(opentracing::string_view s) {
  auto protobuf_header = [&] {
    std::ostringstream oss;
    {
      google::protobuf::io::OstreamOutputStream zero_copy_stream{&oss};
      google::protobuf::io::CodedOutputStream stream{&zero_copy_stream};
      WriteKeyLength<SerializationChain::ReportRequestSpansField>(stream,
                                                                  s.size());
    }
    return oss.str();
  }();
  auto message_size = protobuf_header.size() + s.size();
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase
      << message_size << "\r\n"
      << protobuf_header << s << "\r\n";
  return oss.str();
}
}  // namespace lightstep
